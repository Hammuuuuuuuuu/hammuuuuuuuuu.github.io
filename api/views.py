"""
Django REST Framework Views for Zomato Restaurant API.

Includes root overview view and the 6 revised RESTful API endpoints:
1. Root Overview View (`/`): Displays system info, dependencies, admin credentials, and clickable links.
2. Top-Rated Restaurants (`/api/v1/restaurants/top-rated/`): Multi-table filtering for top-rated restaurants.
3. City Cost Summary Analytics (`/api/v1/analytics/city-cost-summary/`): Aggregates price and rating metrics per city (excluding unrated restaurants).
4. Cuisine Analytics (`/api/v1/cuisines/analytics/`): Calculates popularity and metrics per cuisine filtered by country or city.
5. Multi-Criteria Restaurant Search (`/api/v1/restaurants/search/`): Dynamic multi-table search with filters and pagination.
6. Create Restaurant (`/api/v1/restaurants/` POST): Validates and persists new restaurant and relational data.
7. Restaurant Detail Endpoint (`/api/v1/restaurants/<id>/` GET/PUT/PATCH/DELETE): Nested detail view, full update, partial update, and deletion.
"""

import sys
import django
from django.db.models import Avg, Count, Max, Min, Q
from django.http import HttpResponse
from django.shortcuts import get_object_or_404
from rest_framework import status
from rest_framework.decorators import api_view
from rest_framework.response import Response

from api.models import City, Country, Cuisine, Location, Rating, Restaurant
from api.serializers import (
    CuisineSerializer,
    RestaurantCreateSerializer,
    RestaurantSerializer,
    RestaurantUpdateSerializer,
)


@api_view(["GET"])
def root_overview_view(request):
    """
    Root endpoint (/) detailing Python version, Django version, installed packages,
    admin credentials, and interactive hyperlinks to the 6 API endpoints.
    """
    python_version = sys.version
    django_version = django.get_version()
    packages = [
        "Django==6.1.2",
        "djangorestframework==3.18.3",
        "asgiref==3.13.0",
        "sqlparse==0.6.0",
    ]
    admin_credentials = {
        "username": "admin",
        "password": "adminpassword123",
        "admin_url": "/admin/",
    }

    endpoints = [
        {
            "name": "1. GET /api/v1/restaurants/top-rated/",
            "url": "/api/v1/restaurants/top-rated/?cuisine=Japanese&city=Makati%20City",
            "description": "Multi-table filtering for top-rated restaurants by cuisine, city, country, and table booking availability.",
        },
        {
            "name": "2. GET /api/v1/analytics/city-cost-summary/",
            "url": "/api/v1/analytics/city-cost-summary/?min_restaurants=5",
            "description": "City cost and rating aggregate summary (excluding unrated restaurants).",
        },
        {
            "name": "3. GET /api/v1/cuisines/analytics/",
            "url": "/api/v1/cuisines/analytics/?city=Makati%20City",
            "description": "Cuisine analytics filtered by country or city.",
        },
        {
            "name": "4. GET /api/v1/restaurants/search/",
            "url": "/api/v1/restaurants/search/?q=Grill&has_delivery=true&min_votes=50",
            "description": "Multi-criteria search with keyword, online delivery, price range, and vote threshold filters.",
        },
        {
            "name": "5. POST /api/v1/restaurants/",
            "url": "/api/v1/restaurants/",
            "description": "POST endpoint to validate and save new restaurant records with relational location and rating.",
        },
        {
            "name": "6. GET/PUT/PATCH/DELETE /api/v1/restaurants/<id>/",
            "url": "/api/v1/restaurants/6317637/",
            "description": "Nested detail retrieval, full update (PUT), partial update (PATCH), or deletion (DELETE) for a specific restaurant.",
        },
    ]

    accept_header = request.META.get("HTTP_ACCEPT", "")
    if "text/html" in accept_header or request.GET.get("format") == "html":
        links_html = "".join(
            f'<li><strong><a href="{ep["url"]}">{ep["name"]}</a></strong>: {ep["description"]}</li><br>'
            for ep in endpoints
        )
        pkgs_html = "".join(f"<li>{pkg}</li>" for pkg in packages)

        html_content = f"""
        <!DOCTYPE html>
        <html>
        <head>
            <title>Zomato RESTful API Overview</title>
            <style>
                body {{ font-family: Arial, sans-serif; margin: 30px; line-height: 1.6; color: #333; }}
                h1 {{ color: #cb202d; }}
                .card {{ background: #f9f9f9; padding: 15px; border-radius: 5px; margin-bottom: 20px; border: 1px solid #ddd; }}
                code {{ background: #eee; padding: 2px 5px; border-radius: 3px; }}
            </style>
        </head>
        <body>
            <h1>Zomato RESTful Web Service API</h1>
            <div class="card">
                <h2>System Information & Metadata</h2>
                <p><strong>Python Version:</strong> {python_version}</p>
                <p><strong>Django Version:</strong> {django_version}</p>
                <p><strong>Installed Packages:</strong></p>
                <ul>{pkgs_html}</ul>
                <p><strong>Admin Site Credentials:</strong> Username: <code>{admin_credentials["username"]}</code> | Password: <code>{admin_credentials["password"]}</code> | Link: <a href="{admin_credentials["admin_url"]}">Django Admin Panel</a></p>
            </div>
            <div class="card">
                <h2>REST API Endpoints</h2>
                <ul>
                    {links_html}
                </ul>
            </div>
        </body>
        </html>
        """
        return HttpResponse(html_content, content_type="text/html")

    return Response({
        "system_info": {
            "python_version": python_version,
            "django_version": django_version,
            "packages_used": packages,
            "admin_credentials": admin_credentials,
        },
        "endpoints": endpoints,
    })


@api_view(["GET"])
def top_rated_restaurants_view(request):
    """
    Endpoint 1: Multi-table filtering to retrieve top-rated restaurants.
    Supports query parameters:
    - cuisine: Filter by cuisine name (e.g. Japanese, Italian)
    - city: Filter by city name (e.g. Makati City, New Delhi)
    - country_code: Filter by country integer code
    - has_booking: Filter by table booking availability (true/false)
    - min_rating: Minimum aggregate rating threshold (e.g. 4.0)
    - limit: Max records returned (default: 20)
    """
    queryset = Restaurant.objects.select_related("location__city__country", "rating").prefetch_related("cuisines")

    cuisine = request.GET.get("cuisine")
    if cuisine:
        queryset = queryset.filter(cuisines__name__icontains=cuisine.strip())

    city = request.GET.get("city")
    if city:
        queryset = queryset.filter(location__city__name__icontains=city.strip())

    country_code = request.GET.get("country_code")
    if country_code and country_code.isdigit():
        queryset = queryset.filter(location__city__country__country_code=int(country_code))

    has_booking = request.GET.get("has_booking")
    if has_booking in ["true", "1"]:
        queryset = queryset.filter(has_table_booking=True)
    elif has_booking in ["false", "0"]:
        queryset = queryset.filter(has_table_booking=False)

    min_rating = request.GET.get("min_rating")
    if min_rating:
        try:
            queryset = queryset.filter(rating__aggregate_rating__gte=float(min_rating))
        except ValueError:
            pass

    queryset = queryset.order_by("-rating__aggregate_rating", "-rating__votes")

    try:
        limit = int(request.GET.get("limit", 20))
    except ValueError:
        limit = 20

    serializer = RestaurantSerializer(queryset[:limit], many=True)
    return Response({
        "count": len(serializer.data),
        "results": serializer.data
    })


@api_view(["GET"])
def city_cost_summary_view(request):
    """
    Endpoint 2: Complex aggregation summarizing restaurant pricing and ratings grouped by City,
    EXCLUDING unrated restaurants (aggregate_rating > 0).
    Calculates avg cost for two, min cost, max cost, total rated restaurants, and avg aggregate rating.
    Supports query parameter:
    - min_restaurants: Minimum number of rated restaurants required to include city (default: 1).
    """
    try:
        min_restaurants = int(request.GET.get("min_restaurants", 1))
    except ValueError:
        min_restaurants = 1

    summary = (
        City.objects.filter(locations__restaurants__rating__aggregate_rating__gt=0)
        .values("id", "name", "country__country_code")
        .annotate(
            total_rated_restaurants=Count("locations__restaurants", filter=Q(locations__restaurants__rating__aggregate_rating__gt=0)),
            avg_cost_for_two=Avg("locations__restaurants__average_cost_for_two", filter=Q(locations__restaurants__rating__aggregate_rating__gt=0)),
            min_cost_for_two=Min("locations__restaurants__average_cost_for_two", filter=Q(locations__restaurants__rating__aggregate_rating__gt=0)),
            max_cost_for_two=Max("locations__restaurants__average_cost_for_two", filter=Q(locations__restaurants__rating__aggregate_rating__gt=0)),
            avg_rating=Avg("locations__restaurants__rating__aggregate_rating", filter=Q(locations__restaurants__rating__aggregate_rating__gt=0)),
        )
        .filter(total_rated_restaurants__gte=min_restaurants)
        .order_by("-total_rated_restaurants", "-avg_rating")
    )

    return Response({
        "total_cities": len(summary),
        "cities": summary
    })


@api_view(["GET"])
def cuisine_analytics_view(request):
    """
    Endpoint 3: Complex aggregation analyzing performance metrics across Cuisines per Country or City.
    Supports query parameters:
    - city: Filter cuisines offered by restaurants in a specific city
    - country_code: Filter cuisines offered by restaurants in a specific country code
    - top: Limit top N cuisines (default: 15)
    """
    city_param = request.GET.get("city")
    country_param = request.GET.get("country_code")

    restaurant_filter = Q()
    if city_param:
        restaurant_filter &= Q(restaurants__location__city__name__icontains=city_param.strip())
    if country_param and country_param.isdigit():
        restaurant_filter &= Q(restaurants__location__city__country__country_code=int(country_param))

    analytics = (
        Cuisine.objects.filter(restaurant_filter)
        .annotate(
            restaurant_count=Count("restaurants", filter=restaurant_filter, distinct=True),
            avg_rating=Avg("restaurants__rating__aggregate_rating", filter=restaurant_filter),
            avg_cost=Avg("restaurants__average_cost_for_two", filter=restaurant_filter),
        )
        .filter(restaurant_count__gt=0)
        .order_by("-restaurant_count", "-avg_rating")
    )

    try:
        top = int(request.GET.get("top", 15))
    except ValueError:
        top = 15

    results = []
    for item in analytics[:top]:
        results.append({
            "id": item.id,
            "cuisine_name": item.name,
            "restaurant_count": item.restaurant_count,
            "average_rating": round(item.avg_rating, 2) if item.avg_rating else 0.0,
            "average_cost_for_two": round(item.avg_cost, 2) if item.avg_cost else 0.0,
        })

    filter_info = {}
    if city_param:
        filter_info["city"] = city_param
    if country_param:
        filter_info["country_code"] = country_param

    return Response({
        "filters_applied": filter_info,
        "total_cuisines_analyzed": len(results),
        "cuisines": results
    })


@api_view(["GET"])
def restaurant_search_view(request):
    """
    Endpoint 4: Flexible multi-criteria search for restaurants.
    Supports query parameters:
    - q: Search string matching name, locality, address, or cuisine
    - has_delivery: Filter by online delivery capability (true/false)
    - price_range: Integer price range scale (1 to 4)
    - min_votes: Minimum votes threshold
    - page: Page number (default: 1)
    - page_size: Results per page (default: 20)
    """
    queryset = Restaurant.objects.select_related("location__city__country", "rating").prefetch_related("cuisines")

    query_str = request.GET.get("q")
    if query_str:
        q_clean = query_str.strip()
        queryset = queryset.filter(
            Q(name__icontains=q_clean) |
            Q(location__locality__icontains=q_clean) |
            Q(location__address__icontains=q_clean) |
            Q(cuisines__name__icontains=q_clean)
        ).distinct()

    has_delivery = request.GET.get("has_delivery")
    if has_delivery in ["true", "1"]:
        queryset = queryset.filter(has_online_delivery=True)
    elif has_delivery in ["false", "0"]:
        queryset = queryset.filter(has_online_delivery=False)

    price_range = request.GET.get("price_range")
    if price_range and price_range.isdigit():
        queryset = queryset.filter(price_range=int(price_range))

    min_votes = request.GET.get("min_votes")
    if min_votes and min_votes.isdigit():
        queryset = queryset.filter(rating__votes__gte=int(min_votes))

    try:
        page = max(int(request.GET.get("page", 1)), 1)
    except ValueError:
        page = 1

    try:
        page_size = min(max(int(request.GET.get("page_size", 20)), 1), 100)
    except ValueError:
        page_size = 20

    total_count = queryset.count()
    start = (page - 1) * page_size
    end = start + page_size

    serializer = RestaurantSerializer(queryset[start:end], many=True)
    return Response({
        "total_matches": total_count,
        "page": page,
        "page_size": page_size,
        "results": serializer.data
    })


@api_view(["POST"])
def restaurant_list_create_view(request):
    """
    Endpoint 5: POST endpoint to create a new restaurant.
    Uses RestaurantCreateSerializer for complete payload validation and relational persistence.
    """
    serializer = RestaurantCreateSerializer(data=request.data)
    if serializer.is_valid():
        restaurant = serializer.save()
        read_serializer = RestaurantSerializer(restaurant)
        return Response(
            {
                "message": "Restaurant successfully created.",
                "data": read_serializer.data,
            },
            status=status.HTTP_201_CREATED,
        )
    return Response(serializer.errors, status=status.HTTP_400_BAD_REQUEST)


@api_view(["GET", "PUT", "PATCH", "DELETE"])
def restaurant_detail_view(request, pk):
    """
    Endpoint 6: GET, PUT, PATCH, DELETE endpoint for a specific restaurant by ID.
    - GET: Retrieves nested details for restaurant with specified ID.
    - PUT: Performs a full update of restaurant and nested location/rating/cuisines.
    - PATCH: Performs a partial update of restaurant and nested location/rating/cuisines.
    - DELETE: Deletes restaurant record and associated rating.
    """
    restaurant = get_object_or_404(
        Restaurant.objects.select_related("location__city__country", "rating").prefetch_related("cuisines"),
        restaurant_id=pk
    )

    if request.method == "GET":
        serializer = RestaurantSerializer(restaurant)
        return Response(serializer.data)

    elif request.method in ["PUT", "PATCH"]:
        serializer = RestaurantUpdateSerializer(
            instance=restaurant,
            data=request.data,
            partial=(request.method == "PATCH")
        )
        if serializer.is_valid():
            updated_restaurant = serializer.save()
            read_serializer = RestaurantSerializer(updated_restaurant)
            return Response(
                {
                    "message": "Restaurant successfully updated.",
                    "data": read_serializer.data,
                },
                status=status.HTTP_200_OK
            )
        return Response(serializer.errors, status=status.HTTP_400_BAD_REQUEST)

    elif request.method == "DELETE":
        rating = restaurant.rating
        location = restaurant.location
        restaurant.delete()
        if rating:
            rating.delete()
        if location and location.restaurants.count() == 0:
            location.delete()
        return Response(
            {"message": f"Restaurant with ID {pk} successfully deleted."},
            status=status.HTTP_204_NO_CONTENT
        )

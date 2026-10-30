"""
Django REST Framework Views for Zomato Restaurant API.

Includes root overview view and 6 specialized RESTful API endpoints:
1. Root Overview View (`/`): Displays system info, dependencies, admin credentials, and clickable links.
2. Top-Rated Restaurants (`/api/v1/restaurants/top-rated/`): Multi-table filtering for top-rated restaurants by cuisine, city, and booking features.
3. City Cost Summary Analytics (`/api/v1/analytics/city-cost-summary/`): Aggregates price and rating metrics per city.
4. Cuisine Analytics (`/api/v1/cuisines/analytics/`): Complex aggregation calculating popularity, rating, and average cost per cuisine.
5. Multi-Criteria Restaurant Search (`/api/v1/restaurants/search/`): Dynamic filtering based on name, price range, delivery, and votes.
6. Country Level Analytics (`/api/v1/countries/stats/`): Relational aggregation across country, city, and restaurant metrics.
7. Create Restaurant (`/api/v1/restaurants/create/`): POST endpoint validating and persisting new restaurant and relational data.
"""

import sys
import django
from django.db.models import Avg, Count, Max, Min, Q
from django.http import HttpResponse
from rest_framework import status
from rest_framework.decorators import api_view
from rest_framework.response import Response

from api.models import City, Country, Cuisine, Location, Rating, Restaurant
from api.serializers import (
    CuisineSerializer,
    RestaurantCreateSerializer,
    RestaurantSerializer,
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
            "name": "1. Top Rated Restaurants (Multi-table Filtering)",
            "url": "/api/v1/restaurants/top-rated/?cuisine=Japanese&city=Makati%20City",
            "description": "Find top-rated restaurants filtered by cuisine, city, country, and table booking availability.",
        },
        {
            "name": "2. City Cost & Rating Summary (Aggregation)",
            "url": "/api/v1/analytics/city-cost-summary/?min_restaurants=5",
            "description": "Aggregates min, max, average cost for two and average rating grouped by city.",
        },
        {
            "name": "3. Cuisine Analytics (Aggregation)",
            "url": "/api/v1/cuisines/analytics/?top=10&ordering=-avg_rating",
            "description": "Calculates popularity, average rating, and average cost per cuisine type across all restaurants.",
        },
        {
            "name": "4. Multi-Criteria Restaurant Search",
            "url": "/api/v1/restaurants/search/?q=Grill&has_delivery=true&min_votes=50",
            "description": "Search restaurants with flexible keyword, delivery status, price range, and vote threshold filters.",
        },
        {
            "name": "5. Country Level Statistics",
            "url": "/api/v1/countries/stats/",
            "description": "Relational aggregation detailing total cities, restaurants, and average ratings grouped by country code.",
        },
        {
            "name": "6. Create Restaurant (POST Endpoint)",
            "url": "/api/v1/restaurants/create/",
            "description": "POST endpoint to validate and save new restaurant records with relational location and rating.",
        },
    ]

    # Check if client expects HTML or JSON
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
    Endpoint 2: Complex aggregation summarizing restaurant pricing and ratings grouped by City.
    Calculates avg cost for two, min cost, max cost, total restaurants, and avg aggregate rating.
    Supports query parameter:
    - min_restaurants: Minimum number of restaurants required to include city (default: 1).
    """
    try:
        min_restaurants = int(request.GET.get("min_restaurants", 1))
    except ValueError:
        min_restaurants = 1

    summary = (
        City.objects.values("id", "name", "country__country_code")
        .annotate(
            total_restaurants=Count("locations__restaurants"),
            avg_cost_for_two=Avg("locations__restaurants__average_cost_for_two"),
            min_cost_for_two=Min("locations__restaurants__average_cost_for_two"),
            max_cost_for_two=Max("locations__restaurants__average_cost_for_two"),
            avg_rating=Avg("locations__restaurants__rating__aggregate_rating"),
        )
        .filter(total_restaurants__gte=min_restaurants)
        .order_by("-total_restaurants", "-avg_rating")
    )

    return Response({
        "total_cities": len(summary),
        "cities": summary
    })


@api_view(["GET"])
def cuisine_analytics_view(request):
    """
    Endpoint 3: Complex aggregation analyzing performance metrics across Cuisines.
    Calculates total restaurants, average aggregate rating, and average cost for two per cuisine.
    Supports query parameters:
    - top: Limit top N cuisines (default: 15)
    - ordering: Field to order by (-count, -avg_rating, -avg_cost) (default: -count)
    """
    ordering = request.GET.get("ordering", "-count")
    valid_orderings = {
        "-count": "-restaurant_count",
        "count": "restaurant_count",
        "-avg_rating": "-avg_rating",
        "avg_rating": "avg_rating",
        "-avg_cost": "-avg_cost",
        "avg_cost": "avg_cost",
    }
    order_field = valid_orderings.get(ordering, "-restaurant_count")

    analytics = (
        Cuisine.objects.annotate(
            restaurant_count=Count("restaurants"),
            avg_rating=Avg("restaurants__rating__aggregate_rating"),
            avg_cost=Avg("restaurants__average_cost_for_two"),
        )
        .filter(restaurant_count__gt=0)
        .order_by(order_field)
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

    return Response({
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


@api_view(["GET"])
def country_stats_view(request):
    """
    Endpoint 5: Aggregates metrics across Countries.
    Calculates total cities, total restaurants, overall avg rating, and total votes per country.
    """
    stats = (
        Country.objects.annotate(
            total_cities=Count("cities", distinct=True),
            total_restaurants=Count("cities__locations__restaurants", distinct=True),
            avg_rating=Avg("cities__locations__restaurants__rating__aggregate_rating"),
            total_votes=Min("cities__locations__restaurants__rating__votes"), # Aggregation helper
        )
        .order_by("country_code")
    )

    results = []
    for c in stats:
        # Calculate precise aggregate metrics per country
        rests = Restaurant.objects.filter(location__city__country=c)
        rest_count = rests.count()
        if rest_count > 0:
            avg_r = rests.aggregate(avg=Avg("rating__aggregate_rating"))["avg"] or 0.0
            tot_v = rests.aggregate(tot=Min("rating__votes"))["tot"] or 0
        else:
            avg_r = 0.0
            tot_v = 0

        results.append({
            "id": c.id,
            "country_code": c.country_code,
            "country_name": c.name or f"Country {c.country_code}",
            "total_cities": c.cities.count(),
            "total_restaurants": rest_count,
            "average_rating": round(avg_r, 2),
        })

    return Response({
        "countries_count": len(results),
        "countries": results
    })


@api_view(["POST"])
def create_restaurant_view(request):
    """
    Endpoint 6: POST endpoint to record a new restaurant.
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

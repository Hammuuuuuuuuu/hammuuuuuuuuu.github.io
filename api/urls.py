"""
URL Configuration for API application.

Maps root URL and RESTful API endpoints to their respective view handlers.
"""

from django.urls import path
from api import views

urlpatterns = [
    # Root overview endpoint
    path("", views.root_overview_view, name="root-overview"),

    # RESTful API Endpoints
    path(
        "api/v1/restaurants/top-rated/",
        views.top_rated_restaurants_view,
        name="top-rated-restaurants",
    ),
    path(
        "api/v1/analytics/city-cost-summary/",
        views.city_cost_summary_view,
        name="city-cost-summary",
    ),
    path(
        "api/v1/cuisines/analytics/",
        views.cuisine_analytics_view,
        name="cuisine-analytics",
    ),
    path(
        "api/v1/restaurants/search/",
        views.restaurant_search_view,
        name="restaurant-search",
    ),
    path(
        "api/v1/restaurants/",
        views.restaurant_list_create_view,
        name="restaurant-list-create",
    ),
    path(
        "api/v1/restaurants/<int:pk>/",
        views.restaurant_detail_view,
        name="restaurant-detail",
    ),
]

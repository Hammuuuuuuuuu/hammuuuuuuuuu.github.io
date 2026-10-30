"""
URL Configuration for zomato_project.

Includes admin panel routes and delegates API routing to api.urls.
"""

from django.contrib import admin
from django.urls import include, path

urlpatterns = [
    path("admin/", admin.site.urls),
    path("", include("api.urls")),
]

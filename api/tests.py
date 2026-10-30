"""
Comprehensive Unit Test Suite for Zomato RESTful API.

Tests cover:
- Root Overview view (JSON and HTML representations)
- Endpoint 1: GET /api/v1/restaurants/top-rated/ (multi-table filtering and edge cases)
- Endpoint 2: GET /api/v1/analytics/city-cost-summary/ (grouping and unrated restaurant exclusion)
- Endpoint 3: GET /api/v1/cuisines/analytics/ (country and city filters)
- Endpoint 4: GET /api/v1/restaurants/search/ (keyword, delivery status, pagination)
- Endpoint 5: POST /api/v1/restaurants/ (valid payload creation and error validations)
- Endpoint 6: GET/PUT/PATCH/DELETE /api/v1/restaurants/<id>/ (nested detail, full update, partial update, deletion)
"""

from decimal import Decimal
from django.urls import reverse
from rest_framework import status
from rest_framework.test import APITestCase

from api.models import City, Country, Cuisine, Location, Rating, Restaurant


class ZomatoAPITestCase(APITestCase):
    """
    Test suite verifying all RESTful API endpoints, filtering logic,
    aggregations, validations, and edge cases.
    """

    def setUp(self):
        """Set up test fixture dataset with normalized models."""
        # Countries
        self.country_1 = Country.objects.create(country_code=1, name="India")
        self.country_162 = Country.objects.create(country_code=162, name="Philippines")

        # Cities
        self.city_makati = City.objects.create(name="Makati City", country=self.country_162)
        self.city_delhi = City.objects.create(name="New Delhi", country=self.country_1)

        # Cuisines
        self.cuisine_japanese = Cuisine.objects.create(name="Japanese")
        self.cuisine_italian = Cuisine.objects.create(name="Italian")
        self.cuisine_french = Cuisine.objects.create(name="French")

        # Locations
        self.loc_1 = Location.objects.create(
            city=self.city_makati,
            address="Century Mall",
            locality="Poblacion",
            longitude=121.02,
            latitude=14.56,
        )
        self.loc_2 = Location.objects.create(
            city=self.city_makati,
            address="Little Tokyo",
            locality="Legaspi",
            longitude=121.01,
            latitude=14.55,
        )
        self.loc_3 = Location.objects.create(
            city=self.city_delhi,
            address="Connaught Place",
            locality="CP",
            longitude=77.21,
            latitude=28.63,
        )

        # Ratings
        self.rating_1 = Rating.objects.create(
            aggregate_rating=4.8, rating_color="Dark Green", rating_text="Excellent", votes=300
        )
        self.rating_2 = Rating.objects.create(
            aggregate_rating=4.2, rating_color="Green", rating_text="Very Good", votes=150
        )
        self.rating_3 = Rating.objects.create(
            aggregate_rating=0.0, rating_color="White", rating_text="Not rated", votes=0
        )

        # Restaurants
        self.rest_1 = Restaurant.objects.create(
            restaurant_id=101,
            name="Tokyo Dining",
            location=self.loc_1,
            rating=self.rating_1,
            average_cost_for_two=Decimal("1200.00"),
            currency="PHP",
            has_table_booking=True,
            has_online_delivery=False,
            price_range=3,
        )
        self.rest_1.cuisines.set([self.cuisine_japanese, self.cuisine_french])

        self.rest_2 = Restaurant.objects.create(
            restaurant_id=102,
            name="Sakura Bistro",
            location=self.loc_2,
            rating=self.rating_2,
            average_cost_for_two=Decimal("800.00"),
            currency="PHP",
            has_table_booking=False,
            has_online_delivery=True,
            price_range=2,
        )
        self.rest_2.cuisines.set([self.cuisine_japanese])

        self.rest_3 = Restaurant.objects.create(
            restaurant_id=103,
            name="Pasta Palace",
            location=self.loc_3,
            rating=self.rating_3,
            average_cost_for_two=Decimal("500.00"),
            currency="INR",
            has_table_booking=True,
            has_online_delivery=True,
            price_range=2,
        )
        self.rest_3.cuisines.set([self.cuisine_italian])

    def test_root_overview_view_json(self):
        """Test root endpoint returns JSON metadata and endpoint links."""
        url = reverse("root-overview")
        response = self.client.get(url, HTTP_ACCEPT="application/json")
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.assertIn("system_info", response.data)
        self.assertIn("endpoints", response.data)
        self.assertEqual(len(response.data["endpoints"]), 6)

    def test_root_overview_view_html(self):
        """Test root endpoint returns HTML page when requested by browser."""
        url = reverse("root-overview")
        response = self.client.get(url, HTTP_ACCEPT="text/html")
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.assertEqual(response["Content-Type"], "text/html")
        self.assertIn("Zomato RESTful Web Service API", response.content.decode())

    def test_top_rated_restaurants_filtering(self):
        """Test Endpoint 1 multi-table filtering by cuisine and city."""
        url = reverse("top-rated-restaurants")

        # Filter by Japanese cuisine
        response = self.client.get(url, {"cuisine": "Japanese"})
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.assertEqual(response.data["count"], 2)

        # Filter by city and table booking
        response = self.client.get(url, {"city": "Makati City", "has_booking": "true"})
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.assertEqual(response.data["count"], 1)
        self.assertEqual(response.data["results"][0]["restaurant_id"], 101)

        # Filter by minimum rating
        response = self.client.get(url, {"min_rating": "4.5"})
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.assertEqual(response.data["count"], 1)

    def test_city_cost_summary_excluding_unrated(self):
        """Test Endpoint 2 city cost summary excludes unrated restaurants (rating=0.0)."""
        url = reverse("city-cost-summary")
        response = self.client.get(url)
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.assertIn("cities", response.data)

        # New Delhi (rest_3) is unrated (0.0), so New Delhi should be excluded
        city_names = [c["name"] for c in response.data["cities"]]
        self.assertIn("Makati City", city_names)
        self.assertNotIn("New Delhi", city_names)

        makati_stat = next(c for c in response.data["cities"] if c["name"] == "Makati City")
        self.assertEqual(makati_stat["total_rated_restaurants"], 2)
        self.assertEqual(float(makati_stat["avg_cost_for_two"]), 1000.00)

    def test_cuisine_analytics_per_city_and_country(self):
        """Test Endpoint 3 cuisine metrics aggregation per city or country."""
        url = reverse("cuisine-analytics")

        # Cuisine analytics for Makati City
        response = self.client.get(url, {"city": "Makati City"})
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        cuisines = [c["cuisine_name"] for c in response.data["cuisines"]]
        self.assertIn("Japanese", cuisines)
        self.assertNotIn("Italian", cuisines)

        # Cuisine analytics for Country Code 1 (India)
        response = self.client.get(url, {"country_code": 1})
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        cuisines_in_india = [c["cuisine_name"] for c in response.data["cuisines"]]
        self.assertIn("Italian", cuisines_in_india)
        self.assertNotIn("Japanese", cuisines_in_india)

    def test_restaurant_search(self):
        """Test Endpoint 4 keyword search, delivery filter, and pagination."""
        url = reverse("restaurant-search")

        # Search keyword "Tokyo" (matches "Tokyo Dining" and locality "Little Tokyo")
        response = self.client.get(url, {"q": "Tokyo"})
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.assertEqual(response.data["total_matches"], 2)

        # Search with online delivery and min votes
        response = self.client.get(url, {"has_delivery": "true", "min_votes": 100})
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.assertEqual(response.data["total_matches"], 1)
        self.assertEqual(response.data["results"][0]["restaurant_id"], 102)

    def test_create_restaurant_success(self):
        """Test Endpoint 5 POST creation with valid payload."""
        url = reverse("restaurant-list-create")
        payload = {
            "restaurant_id": 999,
            "name": "Manila Bistro",
            "city_name": "Manila",
            "country_code": 162,
            "address": "Roxas Boulevard",
            "locality": "Malate",
            "cuisines": ["Filipino", "Spanish"],
            "aggregate_rating": 4.6,
            "rating_color": "Dark Green",
            "rating_text": "Excellent",
            "votes": 120,
            "average_cost_for_two": 950.00,
            "currency": "PHP",
            "has_table_booking": True,
            "has_online_delivery": True,
            "price_range": 3,
        }

        response = self.client.post(url, payload, format="json")
        self.assertEqual(response.status_code, status.HTTP_201_CREATED)
        self.assertEqual(response.data["message"], "Restaurant successfully created.")
        self.assertEqual(response.data["data"]["restaurant_id"], 999)

        # Verify record created in database
        self.assertTrue(Restaurant.objects.filter(restaurant_id=999).exists())
        created_rest = Restaurant.objects.get(restaurant_id=999)
        self.assertEqual(created_rest.cuisines.count(), 2)

    def test_create_restaurant_duplicate_id_failure(self):
        """Test Endpoint 5 POST validation error on duplicate restaurant ID."""
        url = reverse("restaurant-list-create")
        payload = {
            "restaurant_id": 101,  # Already exists in setUp
            "name": "Duplicate Restaurant",
            "city_name": "Makati City",
        }

        response = self.client.post(url, payload, format="json")
        self.assertEqual(response.status_code, status.HTTP_400_BAD_REQUEST)
        self.assertIn("restaurant_id", response.data)

    def test_restaurant_detail_get(self):
        """Test Endpoint 6 GET detail retrieval for a specific restaurant ID."""
        url = reverse("restaurant-detail", kwargs={"pk": 101})
        response = self.client.get(url)
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.assertEqual(response.data["restaurant_id"], 101)
        self.assertEqual(response.data["name"], "Tokyo Dining")
        self.assertEqual(response.data["location"]["city_name"], "Makati City")

    def test_restaurant_detail_put(self):
        """Test Endpoint 6 PUT full update for a restaurant."""
        url = reverse("restaurant-detail", kwargs={"pk": 101})
        payload = {
            "name": "Tokyo Fine Dining",
            "city_name": "Makati City",
            "country_code": 162,
            "address": "Updated Address",
            "locality": "Poblacion",
            "aggregate_rating": 4.9,
            "rating_color": "Dark Green",
            "rating_text": "Exceptional",
            "votes": 350,
            "average_cost_for_two": 1500.00,
            "currency": "PHP",
            "has_table_booking": True,
            "has_online_delivery": True,
            "price_range": 4,
            "cuisines": ["Japanese", "Sushi"],
        }
        response = self.client.put(url, payload, format="json")
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.assertEqual(response.data["data"]["name"], "Tokyo Fine Dining")

        self.rest_1.refresh_from_db()
        self.assertEqual(self.rest_1.name, "Tokyo Fine Dining")
        self.assertEqual(self.rest_1.rating.aggregate_rating, 4.9)

    def test_restaurant_detail_patch(self):
        """Test Endpoint 6 PATCH partial update for a restaurant."""
        url = reverse("restaurant-detail", kwargs={"pk": 102})
        payload = {
            "name": "Sakura Japanese Bistro",
            "aggregate_rating": 4.5,
        }
        response = self.client.patch(url, payload, format="json")
        self.assertEqual(response.status_code, status.HTTP_200_OK)

        self.rest_2.refresh_from_db()
        self.assertEqual(self.rest_2.name, "Sakura Japanese Bistro")
        self.assertEqual(self.rest_2.rating.aggregate_rating, 4.5)

    def test_restaurant_detail_delete(self):
        """Test Endpoint 6 DELETE removal of a restaurant record."""
        url = reverse("restaurant-detail", kwargs={"pk": 103})
        response = self.client.delete(url)
        self.assertEqual(response.status_code, status.HTTP_204_NO_CONTENT)

        self.assertFalse(Restaurant.objects.filter(restaurant_id=103).exists())

    def test_restaurant_detail_not_found(self):
        """Test Endpoint 6 returns 404 for non-existent restaurant ID."""
        url = reverse("restaurant-detail", kwargs={"pk": 99999})
        response = self.client.get(url)
        self.assertEqual(response.status_code, status.HTTP_404_NOT_FOUND)

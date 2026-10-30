"""
Comprehensive Unit Test Suite for Zomato RESTful API.

Tests cover:
- Root Overview view (JSON and HTML representations)
- Endpoint 1: Top Rated Restaurants (multi-table filtering and edge cases)
- Endpoint 2: City Cost Summary Analytics (grouping and aggregate calculation)
- Endpoint 3: Cuisine Analytics (ordering, top limit, and performance stats)
- Endpoint 4: Multi-Criteria Search (keyword, delivery status, pagination)
- Endpoint 5: Country Level Statistics (relational metrics aggregation)
- Endpoint 6: Create Restaurant POST endpoint (valid payload creation and error validations)
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
            aggregate_rating=3.5, rating_color="Yellow", rating_text="Good", votes=80
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

    def test_city_cost_summary_aggregation(self):
        """Test Endpoint 2 city cost and rating aggregation analytics."""
        url = reverse("city-cost-summary")
        response = self.client.get(url, {"min_restaurants": 1})
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.assertIn("cities", response.data)

        # Find Makati City aggregate record
        makati_stat = next(c for c in response.data["cities"] if c["name"] == "Makati City")
        self.assertEqual(makati_stat["total_restaurants"], 2)
        self.assertEqual(float(makati_stat["min_cost_for_two"]), 800.00)
        self.assertEqual(float(makati_stat["max_cost_for_two"]), 1200.00)
        self.assertEqual(float(makati_stat["avg_cost_for_two"]), 1000.00)

    def test_cuisine_analytics(self):
        """Test Endpoint 3 cuisine metrics aggregation and ordering."""
        url = reverse("cuisine-analytics")
        response = self.client.get(url, {"ordering": "-avg_rating", "top": 5})
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.assertIn("cuisines", response.data)

        cuisines = response.data["cuisines"]
        self.assertGreater(len(cuisines), 0)
        # Japanese rating avg: (4.8 + 4.2) / 2 = 4.5
        jp_stat = next(c for c in cuisines if c["cuisine_name"] == "Japanese")
        self.assertEqual(jp_stat["restaurant_count"], 2)
        self.assertEqual(jp_stat["average_rating"], 4.5)

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

    def test_country_stats(self):
        """Test Endpoint 5 country level relational metrics aggregation."""
        url = reverse("country-stats")
        response = self.client.get(url)
        self.assertEqual(response.status_code, status.HTTP_200_OK)
        self.assertEqual(response.data["countries_count"], 2)

        ph_stat = next(c for c in response.data["countries"] if c["country_code"] == 162)
        self.assertEqual(ph_stat["total_restaurants"], 2)
        self.assertEqual(ph_stat["average_rating"], 4.5)

    def test_create_restaurant_success(self):
        """Test Endpoint 6 POST creation with valid payload."""
        url = reverse("create-restaurant")
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
        """Test Endpoint 6 POST validation error on duplicate restaurant ID."""
        url = reverse("create-restaurant")
        payload = {
            "restaurant_id": 101,  # Already exists in setUp
            "name": "Duplicate Restaurant",
            "city_name": "Makati City",
        }

        response = self.client.post(url, payload, format="json")
        self.assertEqual(response.status_code, status.HTTP_400_BAD_REQUEST)
        self.assertIn("restaurant_id", response.data)

    def test_create_restaurant_invalid_rating(self):
        """Test Endpoint 6 POST validation error on rating exceeding max value."""
        url = reverse("create-restaurant")
        payload = {
            "restaurant_id": 888,
            "name": "Invalid Rating Diner",
            "city_name": "Quezon City",
            "aggregate_rating": 6.5,  # Max is 5.0
        }

        response = self.client.post(url, payload, format="json")
        self.assertEqual(response.status_code, status.HTTP_400_BAD_REQUEST)
        self.assertIn("aggregate_rating", response.data)

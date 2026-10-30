"""
Django REST Framework Serializers for Zomato API.

Provides data serialization, deserialization, and validation for normalized models:
- CountrySerializer
- CitySerializer
- LocationSerializer
- CuisineSerializer
- RatingSerializer
- RestaurantSerializer (Read / Detail)
- RestaurantCreateSerializer (Write / POST validation)
"""

from rest_framework import serializers
from api.models import City, Country, Cuisine, Location, Rating, Restaurant


class CountrySerializer(serializers.ModelSerializer):
    """Serializer for Country model."""
    class Meta:
        model = Country
        fields = ["id", "country_code", "name"]


class CitySerializer(serializers.ModelSerializer):
    """Serializer for City model including nested country details."""
    country_code = serializers.IntegerField(source="country.country_code", read_only=True)

    class Meta:
        model = City
        fields = ["id", "name", "country", "country_code"]


class LocationSerializer(serializers.ModelSerializer):
    """Serializer for Location model including nested city and country names."""
    city_name = serializers.CharField(source="city.name", read_only=True)
    country_code = serializers.IntegerField(source="city.country.country_code", read_only=True)

    class Meta:
        model = Location
        fields = [
            "id",
            "city",
            "city_name",
            "country_code",
            "address",
            "locality",
            "locality_verbose",
            "longitude",
            "latitude",
        ]


class CuisineSerializer(serializers.ModelSerializer):
    """Serializer for Cuisine model."""
    class Meta:
        model = Cuisine
        fields = ["id", "name"]


class RatingSerializer(serializers.ModelSerializer):
    """Serializer for Rating model."""
    class Meta:
        model = Rating
        fields = ["id", "aggregate_rating", "rating_color", "rating_text", "votes"]


class RestaurantSerializer(serializers.ModelSerializer):
    """
    Read serializer for Restaurant displaying full nested information
    from related Location, City, Rating, and Cuisines models.
    """
    location = LocationSerializer(read_only=True)
    rating = RatingSerializer(read_only=True)
    cuisines = CuisineSerializer(many=True, read_only=True)

    class Meta:
        model = Restaurant
        fields = [
            "restaurant_id",
            "name",
            "location",
            "cuisines",
            "rating",
            "average_cost_for_two",
            "currency",
            "has_table_booking",
            "has_online_delivery",
            "is_delivering_now",
            "switch_to_order_menu",
            "price_range",
        ]


class RestaurantCreateSerializer(serializers.Serializer):
    """
    Serializer for POST endpoint to create a new restaurant along with its
    associated Location, Rating, and Cuisines records with full validation.
    """
    restaurant_id = serializers.IntegerField()
    name = serializers.CharField(max_length=255)

    # City & Location details
    city_name = serializers.CharField(max_length=100)
    country_code = serializers.IntegerField(default=1)
    address = serializers.CharField(max_length=500, required=False, allow_blank=True, default="")
    locality = serializers.CharField(max_length=255, required=False, allow_blank=True, default="")
    locality_verbose = serializers.CharField(max_length=255, required=False, allow_blank=True, default="")
    longitude = serializers.FloatField(required=False, allow_null=True, default=None)
    latitude = serializers.FloatField(required=False, allow_null=True, default=None)

    # Cuisines as list of strings
    cuisines = serializers.ListField(
        child=serializers.CharField(max_length=100),
        required=False,
        default=list
    )

    # Rating details
    aggregate_rating = serializers.FloatField(min_value=0.0, max_value=5.0, default=0.0)
    rating_color = serializers.CharField(max_length=50, required=False, allow_blank=True, default="White")
    rating_text = serializers.CharField(max_length=50, required=False, allow_blank=True, default="Not rated")
    votes = serializers.IntegerField(min_value=0, default=0)

    # Cost & Flags
    average_cost_for_two = serializers.DecimalField(max_digits=12, decimal_places=2, min_value=0, default=0.0)
    currency = serializers.CharField(max_length=50, required=False, allow_blank=True, default="USD")
    has_table_booking = serializers.BooleanField(default=False)
    has_online_delivery = serializers.BooleanField(default=False)
    is_delivering_now = serializers.BooleanField(default=False)
    switch_to_order_menu = serializers.BooleanField(default=False)
    price_range = serializers.IntegerField(min_value=1, max_value=4, default=1)

    def validate_restaurant_id(self, value):
        """Ensure restaurant ID is positive and unique."""
        if value <= 0:
            raise serializers.ValidationError("Restaurant ID must be a positive integer.")
        if Restaurant.objects.filter(restaurant_id=value).exists():
            raise serializers.ValidationError(f"Restaurant with ID {value} already exists.")
        return value

    def validate_name(self, value):
        """Ensure restaurant name is non-empty."""
        if not value.strip():
            raise serializers.ValidationError("Restaurant name cannot be empty.")
        return value.strip()

    def create(self, validated_data):
        """
        Creates related Country, City, Location, Rating, and Cuisine objects,
        then constructs and returns the new Restaurant instance.
        """
        cuisines_data = validated_data.pop("cuisines", [])
        city_name = validated_data.pop("city_name")
        country_code = validated_data.pop("country_code")

        address = validated_data.pop("address", "")
        locality = validated_data.pop("locality", "")
        locality_verbose = validated_data.pop("locality_verbose", "")
        longitude = validated_data.pop("longitude", None)
        latitude = validated_data.pop("latitude", None)

        aggregate_rating = validated_data.pop("aggregate_rating", 0.0)
        rating_color = validated_data.pop("rating_color", "")
        rating_text = validated_data.pop("rating_text", "")
        votes = validated_data.pop("votes", 0)

        # Get or create Country & City
        country, _ = Country.objects.get_or_create(country_code=country_code)
        city, _ = City.objects.get_or_create(name=city_name, country=country)

        # Create Location & Rating
        location = Location.objects.create(
            city=city,
            address=address,
            locality=locality,
            locality_verbose=locality_verbose,
            longitude=longitude,
            latitude=latitude,
        )

        rating = Rating.objects.create(
            aggregate_rating=aggregate_rating,
            rating_color=rating_color,
            rating_text=rating_text,
            votes=votes,
        )

        # Create Restaurant
        restaurant = Restaurant.objects.create(
            location=location,
            rating=rating,
            **validated_data
        )

        # Process Cuisines
        cuisine_objs = []
        for cuisine_name in cuisines_data:
            c_name = cuisine_name.strip()
            if c_name:
                cuisine_obj, _ = Cuisine.objects.get_or_create(name=c_name)
                cuisine_objs.append(cuisine_obj)
        restaurant.cuisines.set(cuisine_objs)

        return restaurant

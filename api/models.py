"""
Normalized Data Models for Zomato Restaurant API.

This module defines the relational database models representing the Zomato dataset:
- Country: Stores unique country codes and country names.
- City: Stores city details associated with a specific Country.
- Location: Stores address, locality, and geographical coordinates.
- Cuisine: Stores distinct cuisine types (e.g., Italian, Japanese).
- Rating: Stores aggregate rating score, color, text label, and vote counts.
- Restaurant: Central model referencing Location, Rating, and Cuisines.
"""

from django.db import models


class Country(models.Model):
    """
    Represents a country where restaurants are located.
    """
    country_code = models.IntegerField(unique=True, db_index=True)
    name = models.CharField(max_length=100, blank=True, default="")

    class Meta:
        verbose_name_plural = "Countries"
        ordering = ["country_code"]

    def __str__(self):
        return f"Country Code {self.country_code}" + (f" ({self.name})" if self.name else "")


class City(models.Model):
    """
    Represents a city located within a country.
    """
    name = models.CharField(max_length=100, db_index=True)
    country = models.ForeignKey(
        Country,
        on_delete=models.CASCADE,
        related_name="cities"
    )

    class Meta:
        verbose_name_plural = "Cities"
        unique_together = ("name", "country")
        ordering = ["name"]

    def __str__(self):
        return f"{self.name}, {self.country.country_code}"


class Location(models.Model):
    """
    Represents detailed physical location and geographical coordinates.
    """
    city = models.ForeignKey(
        City,
        on_delete=models.CASCADE,
        related_name="locations"
    )
    address = models.TextField(blank=True, default="")
    locality = models.CharField(max_length=255, blank=True, default="")
    locality_verbose = models.CharField(max_length=255, blank=True, default="")
    longitude = models.FloatField(null=True, blank=True)
    latitude = models.FloatField(null=True, blank=True)

    class Meta:
        ordering = ["locality"]

    def __str__(self):
        return f"{self.locality}, {self.city.name}"


class Cuisine(models.Model):
    """
    Represents a cuisine type offered by restaurants.
    """
    name = models.CharField(max_length=100, unique=True, db_index=True)

    class Meta:
        ordering = ["name"]

    def __str__(self):
        return self.name


class Rating(models.Model):
    """
    Represents rating metrics and customer feedback votes for a restaurant.
    """
    aggregate_rating = models.FloatField(default=0.0)
    rating_color = models.CharField(max_length=50, blank=True, default="")
    rating_text = models.CharField(max_length=50, blank=True, default="")
    votes = models.IntegerField(default=0)

    class Meta:
        ordering = ["-aggregate_rating", "-votes"]

    def __str__(self):
        return f"{self.aggregate_rating} ({self.rating_text}) - {self.votes} votes"


class Restaurant(models.Model):
    """
    Central Restaurant entity containing core properties, relationships to location,
    cuisines, and ratings.
    """
    restaurant_id = models.BigIntegerField(primary_key=True)
    name = models.CharField(max_length=255, db_index=True)
    location = models.ForeignKey(
        Location,
        on_delete=models.CASCADE,
        related_name="restaurants"
    )
    cuisines = models.ManyToManyField(
        Cuisine,
        related_name="restaurants",
        blank=True
    )
    rating = models.OneToOneField(
        Rating,
        on_delete=models.CASCADE,
        related_name="restaurant"
    )
    average_cost_for_two = models.DecimalField(
        max_digits=12,
        decimal_places=2,
        default=0.0
    )
    currency = models.CharField(max_length=50, blank=True, default="")
    has_table_booking = models.BooleanField(default=False)
    has_online_delivery = models.BooleanField(default=False)
    is_delivering_now = models.BooleanField(default=False)
    switch_to_order_menu = models.BooleanField(default=False)
    price_range = models.IntegerField(default=1)

    class Meta:
        ordering = ["restaurant_id"]

    def __str__(self):
        return f"{self.name} (ID: {self.restaurant_id})"

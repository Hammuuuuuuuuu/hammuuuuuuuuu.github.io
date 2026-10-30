"""
Management command / script to parse ADS/zomato.csv and populate the SQLite database.

Enforces a limit of maximum 10,000 records processed as per specification.
Normalizes data into Country, City, Location, Cuisine, Rating, and Restaurant models.
"""

import csv
import os
import sys
from decimal import Decimal

import django
from django.core.management.base import BaseCommand
from django.db import transaction


def run_seeding(csv_path="ADS/zomato.csv", max_records=10000):
    """
    Parses the CSV file and populates the normalized database tables.

    :param csv_path: Path to the Zomato CSV dataset.
    :param max_records: Maximum number of restaurant records to import.
    """
    from api.models import City, Country, Cuisine, Location, Rating, Restaurant

    if not os.path.exists(csv_path):
        print(f"Error: Dataset file not found at {csv_path}")
        return 0

    print(f"Starting import from {csv_path} (limit: {max_records} records)...")

    # Dictionaries to cache database lookups for speed and normalization
    country_cache = {}
    city_cache = {}
    cuisine_cache = {}

    count = 0

    with open(csv_path, mode="r", encoding="latin-1") as f:
        reader = csv.DictReader(f)

        with transaction.atomic():
            for row in reader:
                if count >= max_records:
                    break

                rest_id = int(row["Restaurant ID"].strip())
                rest_name = row["Restaurant Name"].strip()

                # Country normalization
                country_code = int(row["Country Code"].strip())
                if country_code not in country_cache:
                    country, _ = Country.objects.get_or_create(country_code=country_code)
                    country_cache[country_code] = country
                else:
                    country = country_cache[country_code]

                # City normalization
                city_name = row["City"].strip()
                city_key = (city_name, country.id)
                if city_key not in city_cache:
                    city, _ = City.objects.get_or_create(name=city_name, country=country)
                    city_cache[city_key] = city
                else:
                    city = city_cache[city_key]

                # Location creation
                address = row.get("Address", "").strip()
                locality = row.get("Locality", "").strip()
                locality_verbose = row.get("Locality Verbose", "").strip()

                try:
                    longitude = float(row["Longitude"].strip()) if row.get("Longitude") else None
                except ValueError:
                    longitude = None

                try:
                    latitude = float(row["Latitude"].strip()) if row.get("Latitude") else None
                except ValueError:
                    latitude = None

                location = Location.objects.create(
                    city=city,
                    address=address,
                    locality=locality,
                    locality_verbose=locality_verbose,
                    longitude=longitude,
                    latitude=latitude,
                )

                # Rating creation
                try:
                    agg_rating = float(row.get("Aggregate rating", 0).strip())
                except ValueError:
                    agg_rating = 0.0

                rating_color = row.get("Rating color", "").strip()
                rating_text = row.get("Rating text", "").strip()

                try:
                    votes = int(row.get("Votes", 0).strip())
                except ValueError:
                    votes = 0

                rating = Rating.objects.create(
                    aggregate_rating=agg_rating,
                    rating_color=rating_color,
                    rating_text=rating_text,
                    votes=votes,
                )

                # Cost and flags
                try:
                    cost = Decimal(row.get("Average Cost for two", 0).strip())
                except Exception:
                    cost = Decimal("0.0")

                currency = row.get("Currency", "").strip()
                has_booking = row.get("Has Table booking", "").strip().lower() in ["yes", "1", "true"]
                has_delivery = row.get("Has Online delivery", "").strip().lower() in ["yes", "1", "true"]
                is_delivering = row.get("Is delivering now", "").strip().lower() in ["yes", "1", "true"]
                switch_order = row.get("Switch to order menu", "").strip().lower() in ["yes", "1", "true"]

                try:
                    price_range = int(row.get("Price range", 1).strip())
                except ValueError:
                    price_range = 1

                # Restaurant creation / update
                restaurant, created = Restaurant.objects.update_or_create(
                    restaurant_id=rest_id,
                    defaults={
                        "name": rest_name,
                        "location": location,
                        "rating": rating,
                        "average_cost_for_two": cost,
                        "currency": currency,
                        "has_table_booking": has_booking,
                        "has_online_delivery": has_delivery,
                        "is_delivering_now": is_delivering,
                        "switch_to_order_menu": switch_order,
                        "price_range": price_range,
                    },
                )

                # Cuisines normalization (ManyToMany)
                cuisines_str = row.get("Cuisines", "").strip()
                if cuisines_str:
                    cuisine_list = [c.strip() for c in cuisines_str.split(",") if c.strip()]
                    cuisine_objs = []
                    for c_name in cuisine_list:
                        if c_name not in cuisine_cache:
                            cuisine, _ = Cuisine.objects.get_or_create(name=c_name)
                            cuisine_cache[c_name] = cuisine
                        else:
                            cuisine = cuisine_cache[c_name]
                        cuisine_objs.append(cuisine)
                    restaurant.cuisines.set(cuisine_objs)

                count += 1

    print(f"Successfully imported {count} restaurant records into database.")
    return count


class Command(BaseCommand):
    help = "Parse ADS/zomato.csv and seed SQLite database with restaurant data (max 10,000 records)."

    def add_arguments(self, parser):
        parser.add_argument(
            "--csv",
            type=str,
            default="ADS/zomato.csv",
            help="Path to CSV file (default: ADS/zomato.csv)",
        )
        parser.add_argument(
            "--max-records",
            type=int,
            default=10000,
            help="Maximum records to load (default: 10000)",
        )

    def handle(self, *args, **options):
        csv_path = options["csv"]
        max_records = options["max_records"]
        run_seeding(csv_path, max_records)


if __name__ == "__main__":
    # Allow execution as a standalone script
    sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "../../../")))
    os.environ.setdefault("DJANGO_SETTINGS_MODULE", "zomato_project.settings")
    django.setup()
    run_seeding()

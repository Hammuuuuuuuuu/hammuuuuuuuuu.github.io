
-- This makes sure that foreign_key constraints are observed and that errors will be thrown for violations
PRAGMA foreign_keys=ON;

BEGIN TRANSACTION;

-- Settings table to store site name and description
CREATE TABLE IF NOT EXISTS settings (
    setting_id INTEGER PRIMARY KEY AUTOINCREMENT,
    site_name TEXT NOT NULL,
    site_description TEXT NOT NULL
);

-- Events table to store event details, state, and ticket info
CREATE TABLE IF NOT EXISTS events (
    event_id INTEGER PRIMARY KEY AUTOINCREMENT,
    title TEXT NOT NULL,
    description TEXT,
    event_date DATETIME,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    last_modified_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    published_at DATETIME,
    is_published INTEGER DEFAULT 0, -- 0 for draft, 1 for published
    full_price_tickets_total INTEGER DEFAULT 0,
    full_price_tickets_sold INTEGER DEFAULT 0,
    full_price_tickets_price REAL DEFAULT 0.0,
    concession_tickets_total INTEGER DEFAULT 0,
    concession_tickets_sold INTEGER DEFAULT 0,
    concession_tickets_price REAL DEFAULT 0.0
);

-- Bookings table to store attendee registrations
CREATE TABLE IF NOT EXISTS bookings (
    booking_id INTEGER PRIMARY KEY AUTOINCREMENT,
    event_id INTEGER NOT NULL,
    attendee_name TEXT NOT NULL,
    full_price_tickets_quantity INTEGER DEFAULT 0,
    concession_tickets_quantity INTEGER DEFAULT 0,
    booking_date DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (event_id) REFERENCES events(event_id) ON DELETE CASCADE
);

-- Insert default site settings
INSERT INTO settings (site_name, site_description) VALUES ('Event Manager', 'Welcome to our event management system.');

COMMIT;

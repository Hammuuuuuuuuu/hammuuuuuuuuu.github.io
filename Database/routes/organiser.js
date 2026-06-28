/**
 * organiser.js
 * Routes for the Organiser functionality.
 */

const express = require('express');
const router = express.Router();

/**
 * Route: GET /organiser
 * Purpose: Display the Organiser Home Page with site settings and lists of events.
 * Inputs: None.
 * Outputs: Renders 'organiser-home.ejs' with settings, published events, and draft events.
 */
router.get('/', (req, res) => {
    const settingsQuery = "SELECT * FROM settings LIMIT 1";
    const publishedQuery = "SELECT * FROM events WHERE is_published = 1 ORDER BY published_at DESC";
    const draftQuery = "SELECT * FROM events WHERE is_published = 0 ORDER BY created_at DESC";

    global.db.get(settingsQuery, [], (err, settings) => {
        if (err) return res.status(500).send("Database error");
        global.db.all(publishedQuery, [], (err, publishedEvents) => {
            if (err) return res.status(500).send("Database error");
            global.db.all(draftQuery, [], (err, draftEvents) => {
                if (err) return res.status(500).send("Database error");
                res.render('organiser-home', { settings, publishedEvents, draftEvents });
            });
        });
    });
});

/**
 * Route: GET /organiser/settings
 * Purpose: Display the Site Settings Page with a form.
 * Inputs: None.
 * Outputs: Renders 'site-settings.ejs' with current site settings.
 */
router.get('/settings', (req, res) => {
    const query = "SELECT * FROM settings LIMIT 1";
    global.db.get(query, [], (err, settings) => {
        if (err) return res.status(500).send("Database error");
        res.render('site-settings', { settings });
    });
});

/**
 * Route: POST /organiser/settings
 * Purpose: Update site name and description in the database.
 * Inputs: site_name (string), site_description (string).
 * Outputs: Redirects back to Organiser Home Page.
 */
router.post('/settings', (req, res) => {
    const { site_name, site_description } = req.body;
    if (!site_name || !site_description) return res.status(400).send("Invalid input");

    const query = "UPDATE settings SET site_name = ?, site_description = ? WHERE setting_id = 1";
    global.db.run(query, [site_name, site_description], (err) => {
        if (err) return res.status(500).send("Database error");
        res.redirect('/organiser');
    });
});

/**
 * Route: POST /organiser/create-event
 * Purpose: Create a new draft event and redirect to its edit page.
 * Inputs: None (default values used).
 * Outputs: Redirects to /organiser/edit/:id.
 */
router.post('/create-event', (req, res) => {
    const query = "INSERT INTO events (title, is_published) VALUES (?, ?)";
    global.db.run(query, ["New Event Draft", 0], function(err) {
        if (err) return res.status(500).send("Database error");
        res.redirect(`/organiser/edit/${this.lastID}`);
    });
});

/**
 * Route: GET /organiser/edit/:id
 * Purpose: Display the Edit Event Page for a specific event.
 * Inputs: id (URL parameter).
 * Outputs: Renders 'edit-event.ejs' with current event details.
 */
router.get('/edit/:id', (req, res) => {
    const query = "SELECT * FROM events WHERE event_id = ?";
    global.db.get(query, [req.params.id], (err, event) => {
        if (err) return res.status(500).send("Database error");
        if (!event) return res.status(404).send("Event not found");
        res.render('edit-event', { event });
    });
});

/**
 * Route: POST /organiser/edit/:id
 * Purpose: Update event details and the last modified timestamp.
 * Inputs: id (URL parameter), title, description, event_date, full_price_tickets_total, full_price_tickets_price, concession_tickets_total, concession_tickets_price.
 * Outputs: Redirects back to Organiser Home Page.
 */
router.post('/edit/:id', (req, res) => {
    const { title, description, event_date, full_price_tickets_total, full_price_tickets_price, concession_tickets_total, concession_tickets_price } = req.body;
    const query = `
        UPDATE events
        SET title = ?, description = ?, event_date = ?,
            full_price_tickets_total = ?, full_price_tickets_price = ?,
            concession_tickets_total = ?, concession_tickets_price = ?,
            last_modified_at = CURRENT_TIMESTAMP
        WHERE event_id = ?
    `;
    global.db.run(query, [
        title, description, event_date,
        full_price_tickets_total, full_price_tickets_price,
        concession_tickets_total, concession_tickets_price,
        req.params.id
    ], (err) => {
        if (err) return res.status(500).send("Database error");
        res.redirect('/organiser');
    });
});

/**
 * Route: POST /organiser/publish/:id
 * Purpose: Transition an event from draft to published.
 * Inputs: id (URL parameter).
 * Outputs: Redirects back to Organiser Home Page.
 */
router.post('/publish/:id', (req, res) => {
    const query = "UPDATE events SET is_published = 1, published_at = CURRENT_TIMESTAMP WHERE event_id = ?";
    global.db.run(query, [req.params.id], (err) => {
        if (err) return res.status(500).send("Database error");
        res.redirect('/organiser');
    });
});

/**
 * Route: POST /organiser/delete/:id
 * Purpose: Permanently remove an event from the database.
 * Inputs: id (URL parameter).
 * Outputs: Redirects back to Organiser Home Page.
 */
router.post('/delete/:id', (req, res) => {
    const query = "DELETE FROM events WHERE event_id = ?";
    global.db.run(query, [req.params.id], (err) => {
        if (err) return res.status(500).send("Database error");
        res.redirect('/organiser');
    });
});

module.exports = router;

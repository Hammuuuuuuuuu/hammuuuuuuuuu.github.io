/**
 * attendee.js
 * Routes for the Attendee functionality.
 */

const express = require('express');
const router = express.Router();

/**
 * Route: GET /attendee
 * Purpose: Display the Attendee Home Page with site info and a chronological list of published events.
 * Inputs: None.
 * Outputs: Renders 'attendee-home.ejs' with settings and published events.
 */
router.get('/', (req, res) => {
    const settingsQuery = "SELECT * FROM settings LIMIT 1";
    const eventsQuery = "SELECT * FROM events WHERE is_published = 1 ORDER BY event_date ASC";

    global.db.get(settingsQuery, [], (err, settings) => {
        if (err) return res.status(500).send("Database error");
        global.db.all(eventsQuery, [], (err, events) => {
            if (err) return res.status(500).send("Database error");
            res.render('attendee-home', { settings, events });
        });
    });
});

/**
 * Route: GET /attendee/event/:id
 * Purpose: Display the details of a specific event and a booking form.
 * Inputs: id (URL parameter).
 * Outputs: Renders 'attendee-event.ejs' with event details.
 */
router.get('/event/:id', (req, res) => {
    const query = "SELECT * FROM events WHERE event_id = ? AND is_published = 1";
    global.db.get(query, [req.params.id], (err, event) => {
        if (err) return res.status(500).send("Database error");
        if (!event) return res.status(404).send("Event not found or not published");
        res.render('attendee-event', { event });
    });
});

/**
 * Route: POST /attendee/book/:id
 * Purpose: Handle ticket booking, validate stock, and update the database.
 * Inputs: id (URL parameter), attendee_name (string), full_price_quantity (int), concession_quantity (int).
 * Outputs: Redirects back to the event page or an error message.
 */
router.post('/book/:id', (req, res) => {
    const eventId = req.params.id;
    const { attendee_name, full_price_quantity, concession_quantity } = req.body;
    const fpQty = parseInt(full_price_quantity) || 0;
    const cQty = parseInt(concession_quantity) || 0;

    if (!attendee_name || (fpQty === 0 && cQty === 0)) {
        return res.status(400).send("Attendee name and at least one ticket are required.");
    }

    // Purpose: Check stock and process booking in a "transaction-like" manner.
    global.db.get("SELECT * FROM events WHERE event_id = ?", [eventId], (err, event) => {
        if (err) return res.status(500).send("Database error");
        if (!event) return res.status(404).send("Event not found");

        const fpAvailable = event.full_price_tickets_total - event.full_price_tickets_sold;
        const cAvailable = event.concession_tickets_total - event.concession_tickets_sold;

        if (fpQty > fpAvailable || cQty > cAvailable) {
            return res.status(400).send("Not enough tickets available.");
        }

        // Update sold counts in events table
        const updateEventQuery = `
            UPDATE events
            SET full_price_tickets_sold = full_price_tickets_sold + ?,
                concession_tickets_sold = concession_tickets_sold + ?
            WHERE event_id = ?
        `;
        global.db.run(updateEventQuery, [fpQty, cQty, eventId], (err) => {
            if (err) return res.status(500).send("Database error");

            // Insert booking record
            const insertBookingQuery = `
                INSERT INTO bookings (event_id, attendee_name, full_price_tickets_quantity, concession_tickets_quantity)
                VALUES (?, ?, ?, ?)
            `;
            global.db.run(insertBookingQuery, [eventId, attendee_name, fpQty, cQty], (err) => {
                if (err) return res.status(500).send("Database error");
                res.redirect(`/attendee/event/${eventId}`);
            });
        });
    });
});

module.exports = router;

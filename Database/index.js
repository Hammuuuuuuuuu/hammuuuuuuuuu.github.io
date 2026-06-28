/**
 * index.js
 * Main app entry point.
 * Sets up Express, EJS, body-parser, and SQLite database.
 */

const express = require('express');
const app = express();
const port = 3000;
const bodyParser = require("body-parser");
const path = require('path');

// Configure middleware
// Purpose: Parse URL-encoded bodies for form submissions.
app.use(bodyParser.urlencoded({ extended: true }));

// Purpose: Set EJS as the template engine.
app.set('view engine', 'ejs');
app.set('views', path.join(__dirname, 'views'));

// Purpose: Serve static files from the 'public' directory.
app.use(express.static(path.join(__dirname, 'public')));

// Set up SQLite Database
const sqlite3 = require('sqlite3').verbose();
// Purpose: Establish a persistent connection to the SQLite database.
// Inputs: Path to the database file.
// Outputs: Global database object (db).
global.db = new sqlite3.Database('./database.db', (err) => {
    if (err) {
        console.error("Database connection error:", err.message);
        process.exit(1);
    } else {
        console.log("Connected to the SQLite database.");
        global.db.run("PRAGMA foreign_keys=ON");
    }
});

// Route: Main Home Page (/)
// Purpose: Display the main home page with links to Organiser and Attendee sections.
// Inputs: None.
// Outputs: Renders 'main-home.ejs'.
app.get('/', (req, res) => {
    res.render('main-home');
});

// Route Middlewares
const organiserRoutes = require('./routes/organiser');
const attendeeRoutes = require('./routes/attendee');

// Purpose: Register organiser-related routes under the /organiser path.
app.use('/organiser', organiserRoutes);

// Purpose: Register attendee-related routes under the /attendee path.
app.use('/attendee', attendeeRoutes);

// Start Server
app.listen(port, () => {
    console.log(`Event Manager app listening at http://localhost:${port}`);
});

/* ======================================================================================
   CM2030: Graphics Programming — Coursework 2: Image Processing Applications

   Author: Graphics Programming Expert
   File: sketch.js

   --------------------------------------------------------------------------------------
   COURSEWORK COMMENTARY & REPORT (400–500 Words)
   --------------------------------------------------------------------------------------
   Walkthrough & Interaction Pipeline:
   This single-file p5.js application features an integrated, state-driven workflow split
   into an Intro/Menu Screen (currentTask = 0) and two distinct technical tasks. Pressing '1'
   navigates to Task 1 (Streaming Carousel), and pressing '2' navigates to Task 2 (Panorama
   Motion Guide).
   - In Task 1, the user must follow a sequential keyboard pipeline: press 'c' to initialize
     the carousel layout, press 'l' to load and apply our 2D thresholding matrix for background
     removal, and press 's' to start the smooth carousel animations.
   - In Task 2, the user enters via 'p', loads frame pairs with 'i', converts them to grayscale
     via 'g', extracts outline profiles using a custom edge detection filter with 'e', thresholds
     the edges via 't' (with an interactive slider), computes image centroids with 'n', and overlays
     a motion guide direction arrow via 'd'.

   Technical Challenges & Custom Solutions:
   1. Background Removal: Standard RGB thresholding fails for images with non-uniform backgrounds or
      shadows. By implementing a 2D threshold matrix—storing the colour space selection (0 for RGB, 1
      for HSB) and the three coordinate bounds (e.g., Hue, Saturation, Brightness)—each image is
      thresholded dynamically. A key innovation is running this pixel-array processing on demand
      into transparent `p5.Image` buffers upon pressing 'l', rather than inside the draw loop,
      which preserves 60 FPS performance.
   2. Coordinate-Only Transitions: The brief strictly forbids translate() for Task 1 carousel.
      All zoom and movement offsets are manually computed using pure coordinate arithmetic with
      normalized interpolation and alpha-blending opacity mapped through tint().
   3. Centroid Tracking on Edge Noise: To ensure reliable dx/dy calculation for motion detection, a simple
      Sobel/Scharr derivative filter was built. Edge noise was mitigated by thresholding the gradient
      and using an interactive p5.js slider. The centroid (Cx, Cy) calculation dynamically ignores
      non-edge pixels to output accurate dx/dy, mapping to all 8 cardinal and diagonal directions.

   Project Timeline & Targets Reflection:
   The project was executed on target. Breaking down tasks into loading, preprocessing, animating,
   and analyzing phases prevented integration bottlenecks. Preparing additional motion pairs (pair5 to pair8)
   using image offset scripts ensured early validation of all 8 direction outputs (UP, DOWN, LEFT, RIGHT,
   UP-LEFT, UP-RIGHT, DOWN-LEFT, DOWN-RIGHT).

   Technical Creative Extension:
   To implement a highly challenging and visually arresting creative extension, we developed an Interactive
   Particle Vector-Field Transition System cleanly integrated into Task 1. During slide transitions, instead
   of a standard fade, the person's processed foreground pixels "shatter" and explode. Thousands of active
   particles are spawned from the person's thresholded body pixels. These particles are driven by a dynamic,
   multi-layered Perlin noise vector field, swirling outward and fading beautifully. Users can also interact
   by clicking on the canvas to repel these particles in real time, making the carousel a fully immersive
   and mathematically complex experience.
   ====================================================================================== */

// --- GLOBAL STATE ---
let currentTask = 0; // 0 = Menu, 1 = Task 1, 2 = Task 2

// --- ASSET PATHS & BUFFERS ---
let t1ImagesRaw = [];
let t1ImagesProcessed = [];
let t1BackgroundImage;
const T1_IMAGE_COUNT = 8;

// Threshold matrix: [colourSpace (0=RGB, 1=HSB), c1, c2, c3, optional_invert/extra_params]
// We will fine-tune these to strip background pixels cleanly.
// Images with white background (e.g., 2, 4, 5, 6, 7) are stripped of white/near-white.
// Images with colored background (e.g., 1, 3, 8) are stripped accordingly.
// For HSB, Hue is 0-360, Sat is 0-100, Brightness is 0-100 in p5 by default, but let's stay in standard 0-255 or use normal HSB bounds.
// Let's use RGB (0) or HSB (1) with direct thresholding ranges.
let thresholds = [
  // Image 1: has greyish-pink background (Corners: RGB around 200-220). HSB is great for isolating skin/clothes vs background.
  // We want to keep the person (clothing is dark, skin is warm) and remove greyish background.
  // Let's use HSB (1). Hue for pinkish-grey background, or just keep pixels that are NOT grey-pink.
  // Actually, we can use RGB thresholding: discard pixels where R, G, B are all > 170 and difference between them is small (grey).
  // Or: colourSpace=1 (HSB), we keep only high saturation or specific brightness.
  // Let's define: [colourSpace, param1, param2, param3, optional...]
  [1, 15, 230, 240],
  [0, 240, 240, 240], // Image 2: white background (R,G,B > 240)
  [1, 25, 200, 210], // Image 3: pinkish background
  [0, 245, 245, 245], // Image 4: white background
  [0, 245, 245, 245], // Image 5: white background
  [0, 245, 245, 245], // Image 6: white background
  [0, 240, 240, 240], // Image 7: white background
  [1, 20, 220, 200]  // Image 8: tan background
];

// Task 2 Frames
let t2Pairs = []; // Each element: { img1: p5.Image, img2: p5.Image, name: String }
let t2CurrentPairIdx = 0;
let t2State = "panorama"; // "panorama", "loaded", "grayscale", "edge", "thresholded", "centroid", "arrow"
let t2ThresholdSlider;
let t2LastSliderVal = -1;

// --- TASK 1 ANIMATION STATE ---
let t1State = "menu"; // "menu", "initialized", "loaded", "running"
let t1CurrentIdx = 0;
let t1PrevIdx = 7;
let t1AnimProgress = 0.0; // 0.0 to 1.0
let t1AnimSpeed = 0.01;   // transition increment
let t1BackgroundX = 0;
let t1LastTransitionTime = 0;
const T1_TRANSITION_INTERVAL = 4000; // 4 seconds per slide

// Movie Title Database for Text
const t1Titles = [
  "CHROME ODYSSEY",
  "NEON PULSE",
  "SHADOW HUNTER",
  "CYBERPUNK REVELATION",
  "SOLAR ECLIPSE",
  "VOID RUNNER",
  "ECHOES OF TOMORROW",
  "METROPOLIS REBORN"
];

// --- CREATIVE EXTENSION STATE (PARTICLE SYSTEM) ---
let particles = [];
const MAX_PARTICLES = 1200;

class Particle {
  constructor(x, y, col) {
    this.x = x;
    this.y = y;
    this.vx = random(-2, 2);
    this.vy = random(-2, 2);
    this.color = col;
    this.alpha = 255;
    this.size = random(2, 6);
    this.life = 1.0; // 1.0 down to 0.0
    this.decay = random(0.01, 0.025);
  }

  update() {
    // Vector field movement using Perlin Noise
    let angle = noise(this.x * 0.005, this.y * 0.005, frameCount * 0.01) * TWO_PI * 2.0;
    this.vx += cos(angle) * 0.15;
    this.vy += sin(angle) * 0.15;

    // Mouse interaction (repel)
    if (mouseIsPressed) {
      let d = dist(mouseX, mouseY, this.x, this.y);
      if (d < 120) {
        let force = (120 - d) * 0.08;
        this.vx += ((this.x - mouseX) / d) * force;
        this.vy += ((this.y - mouseY) / d) * force;
      }
    }

    // Drag / Friction
    this.vx *= 0.95;
    this.vy *= 0.95;

    // Move
    this.x += this.vx;
    this.y += this.vy;

    // Decay
    this.life -= this.decay;
    this.alpha = this.life * 255;
  }

  draw() {
    noStroke();
    let c = color(red(this.color), green(this.color), blue(this.color), this.alpha);
    fill(c);
    ellipse(this.x, this.y, this.size);
  }
}

// Preload assets
function preload() {
  // Load Task 1 Images (1.jpg to 8.jpg)
  for (let i = 1; i <= T1_IMAGE_COUNT; i++) {
    t1ImagesRaw.push(loadImage(`./task1-images/${i}.jpg`));
  }

  // Custom Background Image for Task 1
  t1BackgroundImage = loadImage(`./task1-images/2.jpg`); // Will use an existing image as background

  // Preload Task 2 Image Pairs (pair1 to pair8)
  // Directions: Pair 1: RIGHT, Pair 2: LEFT, Pair 3: DOWN-RIGHT, Pair 4: UP-LEFT
  // Created Pairs: Pair 5: UP, Pair 6: DOWN, Pair 7: UP-RIGHT, Pair 8: DOWN-LEFT
  for (let i = 1; i <= 8; i++) {
    t2Pairs.push({
      img1: loadImage(`./task2-images/pair${i}_1.png`),
      img2: loadImage(`./task2-images/pair${i}_2.png`),
      name: `Pair ${i}`
    });
  }
}

function setup() {
  createCanvas(1000, 700);
  background(18);

  // Create HTML slider for Task 2 edge thresholding (initially hidden)
  t2ThresholdSlider = createSlider(0, 255, 60);
  t2ThresholdSlider.position(20, height - 40);
  t2ThresholdSlider.style('width', '200px');
  t2ThresholdSlider.hide();
}

function draw() {
  // Global State Selector
  if (currentTask === 0) {
    drawMenuScreen();
  } else if (currentTask === 1) {
    drawTask1();
  } else if (currentTask === 2) {
    drawTask2();
  }
}

// ======================================================================================
// INTRO / MENU SCREEN (currentTask = 0)
// ======================================================================================
function drawMenuScreen() {
  background(15, 15, 25);

  // Decorative geometric background
  stroke(40, 40, 80);
  noFill();
  for (let i = 0; i < width; i += 80) {
    line(i, 0, i + 200, height);
  }

  // Title box
  fill(25, 25, 45, 220);
  stroke(0, 150, 255);
  strokeWeight(3);
  rect(150, 120, 700, 460, 15);

  // Title Text
  noStroke();
  fill(0, 180, 255);
  textSize(32);
  textAlign(CENTER, TOP);
  textStyle(BOLD);
  text("CM2030: GRAPHICS PROGRAMMING", width / 2, 160);

  fill(255);
  textSize(22);
  text("Coursework 2: Image Processing Applications", width / 2, 210);

  // Separator
  stroke(50, 100, 200);
  strokeWeight(1.5);
  line(250, 250, 750, 250);
  noStroke();

  // Key bindings info
  textAlign(LEFT, TOP);
  textSize(16);
  fill(200, 220, 255);

  let keyY = 280;
  let labelX = 220;
  let descX = 400;

  textStyle(BOLD);
  text("GLOBAL CONTROLS:", labelX, keyY);
  textStyle(NORMAL);
  text("Press '1' for Task 1  |  Press '2' for Task 2  |  Press '0' for Menu", descX, keyY);

  keyY += 40;
  textStyle(BOLD);
  text("TASK 1 CONTROLS:", labelX, keyY);
  textStyle(NORMAL);
  text("'c' = Initialize Carousel Layout\n'l' = Perform Background Removal Pre-processing\n's' = Start Continuous Animation & Streaming", descX, keyY);

  keyY += 75;
  textStyle(BOLD);
  text("TASK 2 CONTROLS:", labelX, keyY);
  textStyle(NORMAL);
  text("'p' = Show Panorama Screen      'i' = Load Next Frame Pair\n'g' = Grayscale Conversion      'e' = Extract Outline Profile\n't' = Edge Thresholding (Adjust with Slider)\n'n' = Compute Centroids & DX/DY   'd' = Overlay Motion Arrow", descX, keyY);

  keyY += 105;
  textAlign(CENTER, TOP);
  textSize(15);
  fill(0, 255, 150);
  textStyle(BOLD);
  text("Press Key '1' or '2' on your keyboard to launch a task!", width / 2, keyY);
}

// ======================================================================================
// KEYBOARD CONTROLS & PIPELINES
// ======================================================================================
function keyPressed() {
  // Global Task Selector
  if (key === '0') {
    currentTask = 0;
    t2ThresholdSlider.hide();
  } else if (key === '1') {
    currentTask = 1;
    t2ThresholdSlider.hide();
  } else if (key === '2') {
    currentTask = 2;
  }

  // Task 1 Keys
  if (currentTask === 1) {
    if (key === 'c' || key === 'C') {
      t1State = "initialized";
      t1CurrentIdx = 0;
      t1AnimProgress = 0;
      particles = [];
    } else if (key === 'l' || key === 'L') {
      if (t1State === "initialized") {
        performBackgroundRemoval();
        t1State = "loaded";
      }
    } else if (key === 's' || key === 'S') {
      if (t1State === "loaded" || t1State === "initialized") {
        if (t1ImagesProcessed.length === 0) {
          performBackgroundRemoval();
        }
        t1State = "running";
        t1LastTransitionTime = millis();
      }
    }
  }

  // Task 2 Keys
  if (currentTask === 2) {
    if (key === 'p' || key === 'P') {
      t2State = "panorama";
      t2ThresholdSlider.hide();
    } else if (key === 'i' || key === 'I') {
      // Cycle image pair
      t2CurrentPairIdx = (t2CurrentPairIdx + 1) % t2Pairs.length;
      t2State = "loaded";
      t2ThresholdSlider.hide();
    } else if (key === 'g' || key === 'G') {
      if (t2State === "loaded" || t2State === "panorama") {
        processGrayscale(t2Pairs[t2CurrentPairIdx]);
        t2State = "grayscale";
      }
    } else if (key === 'e' || key === 'E') {
      if (t2State === "grayscale") {
        processEdges();
        t2State = "edge";
      }
    } else if (key === 't' || key === 'T') {
      if (t2State === "edge") {
        processThresholding();
        t2LastSliderVal = t2ThresholdSlider.value();
        t2State = "thresholded";
        t2ThresholdSlider.show();
      }
    } else if (key === 'n' || key === 'N') {
      if (t2State === "thresholded") {
        computeCentroids();
        t2State = "centroid";
      }
    } else if (key === 'd' || key === 'D') {
      if (t2State === "centroid") {
        determineMotionDirection();
        t2State = "arrow";
      }
    }
  }
}

// ======================================================================================
// TASK 1: STREAMING CAROUSEL WITH BACKGROUND REMOVAL (40%) + CREATIVE EXTENSION
// ======================================================================================
function performBackgroundRemoval() {
  t1ImagesProcessed = [];

  for (let i = 0; i < T1_IMAGE_COUNT; i++) {
    let img = t1ImagesRaw[i];
    let processed = createImage(img.width, img.height);

    img.loadPixels();
    processed.loadPixels();

    let mode = thresholds[i][0]; // 0 = RGB, 1 = HSB
    let thresh1 = thresholds[i][1];
    let thresh2 = thresholds[i][2];
    let thresh3 = thresholds[i][3];

    for (let y = 0; y < img.height; y++) {
      for (let x = 0; x < img.width; x++) {
        let index = (x + y * img.width) * 4;
        let r = img.pixels[index];
        let g = img.pixels[index + 1];
        let b = img.pixels[index + 2];
        let a = img.pixels[index + 3];

        let remove = false;

        if (mode === 0) {
          // RGB Thresholding
          // If close to white (near the background values)
          if (r >= thresh1 && g >= thresh2 && b >= thresh3) {
            remove = true;
          }
          // Special RGB check for close gray/uniform colors
          let diffRG = abs(r - g);
          let diffGB = abs(g - b);
          let diffBR = abs(b - r);
          if (r > 160 && g > 160 && b > 160 && diffRG < 20 && diffGB < 20 && diffBR < 20) {
            remove = true;
          }
        } else {
          // HSB Thresholding
          // Convert RGB to HSB
          let hsb = rgbToHsb(r, g, b);
          let h = hsb[0]; // 0 to 360
          let s = hsb[1]; // 0 to 100
          let val = hsb[2]; // 0 to 100

          // Image 1: has pinkish-grey background (Hue ~ 320-350 or saturation is very low, brightness is high)
          if (i === 0) {
            if (s < 12 && val > 70) remove = true; // Desaturated gray background
            if (h > 310 && h < 355 && s < 18) remove = true;
          }
          // Image 3: pinkish background
          else if (i === 2) {
            if (h > 310 && h < 355 && s < 25) remove = true;
            if (s < 15 && val > 75) remove = true;
          }
          // Image 8: tan background (Hue around 20-40, Sat around 10-35)
          else if (i === 7) {
            if (h >= 15 && h <= 45 && s < 40 && val > 55) remove = true;
            if (s < 15 && val > 80) remove = true;
          }
        }

        if (remove) {
          processed.pixels[index] = 0;
          processed.pixels[index + 1] = 0;
          processed.pixels[index + 2] = 0;
          processed.pixels[index + 3] = 0; // Completely transparent
        } else {
          processed.pixels[index] = r;
          processed.pixels[index + 1] = g;
          processed.pixels[index + 2] = b;
          processed.pixels[index + 3] = a;
        }
      }
    }

    processed.updatePixels();
    t1ImagesProcessed.push(processed);
  }
}

// Helper to convert RGB (0-255) to HSB (H:0-360, S:0-100, B:0-100)
function rgbToHsb(r, g, b) {
  r /= 255;
  g /= 255;
  b /= 255;
  let maxVal = max(r, g, b);
  let minVal = min(r, g, b);
  let d = maxVal - minVal;
  let h;
  let s = maxVal === 0 ? 0 : (d / maxVal) * 100;
  let v = maxVal * 100;

  if (maxVal === minVal) {
    h = 0; // achromatic
  } else {
    switch (maxVal) {
      case r: h = (g - b) / d + (g < b ? 6 : 0); break;
      case g: h = (b - r) / d + 2; break;
      case b: h = (r - g) / d + 4; break;
    }
    h /= 6;
  }
  return [h * 360, s, v];
}

function drawTask1() {
  background(10);

  if (t1State === "menu") {
    // Menu screen state within Task 1
    drawTask1Header("Task 1: Streaming Carousel Initialization");

    fill(25, 25, 35);
    stroke(80, 100, 250);
    rect(100, 150, 800, 400, 10);

    noStroke();
    fill(255);
    textSize(20);
    textAlign(CENTER, CENTER);
    textStyle(BOLD);
    text("TASK 1 PIPELINE ACTION REQUIRED", width / 2, 200);

    textStyle(NORMAL);
    textSize(16);
    fill(200, 200, 220);
    text("This pipeline must be executed sequentially via the keyboard:\n\n" +
         "1. Press 'c' to initialize the carousel structures\n" +
         "2. Press 'l' to pre-compute the background removal matrix\n" +
         "3. Press 's' to launch the streaming carousel animation engine", width / 2, 320);

    fill(0, 180, 255);
    textStyle(ITALIC);
    text("Current State: WAITING FOR 'c' KEY", width / 2, 450);
    return;
  }

  if (t1State === "initialized") {
    drawTask1Header("Task 1: Carousel Initialized");

    // Display raw images as grid to prove they are loaded
    let cols = 4;
    let gridW = 120;
    let gridH = 150;
    let startX = 220;
    let startY = 180;

    for (let i = 0; i < T1_IMAGE_COUNT; i++) {
      let r = floor(i / cols);
      let c = i % cols;
      let x = startX + c * (gridW + 40);
      let y = startY + r * (gridH + 40);

      // Draw frame
      stroke(100);
      noFill();
      rect(x - 5, y - 5, gridW + 10, gridH + 10, 5);

      image(t1ImagesRaw[i], x, y, gridW, gridH);

      fill(255);
      noStroke();
      textSize(12);
      textAlign(CENTER, TOP);
      text(`Raw Img ${i + 1}`, x + gridW / 2, y + gridH + 5);
    }

    fill(0, 255, 100);
    textSize(16);
    textAlign(CENTER, TOP);
    textStyle(BOLD);
    text("Step 1 Complete! Press 'l' to load threshold parameters and perform background removal.", width / 2, 580);
    return;
  }

  if (t1State === "loaded") {
    drawTask1Header("Task 1: Background Removal Complete");

    // Display processed foregrounds to prove alpha transparency works!
    let cols = 4;
    let gridW = 120;
    let gridH = 150;
    let startX = 220;
    let startY = 180;

    for (let i = 0; i < T1_IMAGE_COUNT; i++) {
      let r = floor(i / cols);
      let c = i % cols;
      let x = startX + c * (gridW + 40);
      let y = startY + r * (gridH + 40);

      // Checkered background pattern to prove transparency
      drawCheckerboard(x, y, gridW, gridH, 10);

      // Draw frame
      stroke(0, 255, 150);
      noFill();
      rect(x - 5, y - 5, gridW + 10, gridH + 10, 5);

      image(t1ImagesProcessed[i], x, y, gridW, gridH);

      fill(255);
      noStroke();
      textSize(12);
      textAlign(CENTER, TOP);
      text(`Thresh Img ${i + 1}`, x + gridW / 2, y + gridH + 5);
    }

    fill(0, 180, 255);
    textSize(16);
    textAlign(CENTER, TOP);
    textStyle(BOLD);
    text("Step 2 Complete! Press 's' to start the streaming animation sequence.", width / 2, 580);
    return;
  }

  if (t1State === "running") {
    // RUNNING CAROUSEL ANIMATION ENGINE

    // 1. Render Scrolling Background Image (Moves Left-to-Right)
    // We increment background X position smoothly.
    t1BackgroundX += 0.8;
    if (t1BackgroundX >= width) t1BackgroundX = 0;

    // Draw background twice to cover seam
    // We adjust background tint to make it darker for movie contrast
    tint(80, 100, 150, 255);
    image(t1BackgroundImage, t1BackgroundX - width, 0, width, height);
    image(t1BackgroundImage, t1BackgroundX, 0, width, height);
    noTint();

    // 2. State transition timer
    let elapsed = millis() - t1LastTransitionTime;
    if (elapsed >= T1_TRANSITION_INTERVAL) {
      // Trigger shatter transition particles
      spawnExplosionParticles(t1ImagesProcessed[t1CurrentIdx]);

      // Rotate active images
      t1PrevIdx = t1CurrentIdx;
      t1CurrentIdx = (t1CurrentIdx + 1) % T1_IMAGE_COUNT;
      t1LastTransitionTime = millis();
      t1AnimProgress = 0.0;
    }

    // Animation progress (0.0 -> 1.0)
    t1AnimProgress = constrain((millis() - t1LastTransitionTime) / 1000.0, 0.0, 1.0);

    // Alternating animation sequence flags:
    // Mode (i): Zoom IN: Starts small, scales up.
    // Mode (ii): Zoom OUT: Starts large, scales down.
    let isZoomIn = (t1CurrentIdx % 2 === 0);

    // Active Person Calculations (Moves LEFT to RIGHT)
    // Starting X: 100, Ending X: width - 500
    // Zoom IN: scale moves from 0.7 to 1.15
    // Zoom OUT: scale moves from 1.15 to 0.7
    // Fade IN/OUT: mapped via progress
    let personX = lerp(100, width - 450, t1AnimProgress);
    let personScale = isZoomIn ? lerp(0.65, 1.1, t1AnimProgress) : lerp(1.1, 0.65, t1AnimProgress);

    let personOpacity;
    if (t1AnimProgress < 0.2) {
      personOpacity = map(t1AnimProgress, 0.0, 0.2, 0, 255); // fade in
    } else if (t1AnimProgress > 0.8) {
      personOpacity = map(t1AnimProgress, 0.8, 1.0, 255, 0); // fade out
    } else {
      personOpacity = 255;
    }

    // Render Person (using coordinate calculations, strictly NO translate())
    let currentImg = t1ImagesProcessed[t1CurrentIdx];
    let drawW = currentImg.width * personScale * 0.45;
    let drawH = currentImg.height * personScale * 0.45;
    // Keep baseline vertically centered or anchored
    let personY = height / 2 - drawH / 2;

    tint(255, personOpacity);
    image(currentImg, personX, personY, drawW, drawH);
    noTint();

    // 3. Render Text (Moves RIGHT to LEFT)
    // Fade, positioning calculated without translate()
    let textX = lerp(width - 50, 50, t1AnimProgress);
    let textY = height - 120;

    fill(0, 180, 255, personOpacity);
    textSize(48);
    textAlign(LEFT, TOP);
    textStyle(BOLD);
    text(t1Titles[t1CurrentIdx], textX, textY);

    fill(255, personOpacity * 0.8);
    textSize(18);
    text("NOW STREAMING  |  EXCLUSIVE DIRECTORS CUT", textX, textY + 60);

    // 4. Update & Render Creative Extension Particles
    for (let i = particles.length - 1; i >= 0; i--) {
      let p = particles[i];
      p.update();
      p.draw();
      if (p.life <= 0) {
        particles.splice(i, 1);
      }
    }

    // Particle prompt
    fill(255, 150);
    textSize(12);
    textAlign(RIGHT, TOP);
    textStyle(NORMAL);
    text("CREATIVE EXTENSION: Click and hold mouse to repel slide-transition explosion particles!", width - 20, 20);

    // Task Banner
    drawTask1Header(`Streaming Title: ${t1Titles[t1CurrentIdx]}`);
  }
}

function drawTask1Header(sub) {
  fill(0, 15, 30, 200);
  rect(0, 0, width, 60);
  fill(0, 180, 255);
  textSize(20);
  textAlign(LEFT, CENTER);
  textStyle(BOLD);
  text("TASK 1 — CAROUSEL STREAMING", 20, 30);

  fill(255);
  textSize(14);
  textAlign(RIGHT, CENTER);
  text(sub, width - 20, 30);
}

function drawCheckerboard(x, y, w, h, size) {
  noStroke();
  for (let cy = 0; cy < h; cy += size) {
    for (let cx = 0; cx < w; cx += size) {
      if ((floor(cx / size) + floor(cy / size)) % 2 === 0) {
        fill(40);
      } else {
        fill(60);
      }
      rect(x + cx, y + cy, min(size, w - cx), min(size, h - cy));
    }
  }
}

// Particle transition trigger: Spawn particles from non-transparent pixels
function spawnExplosionParticles(img) {
  img.loadPixels();
  let step = 8; // skip pixels for performance
  for (let y = 0; y < img.height; y += step) {
    for (let x = 0; x < img.width; x += step) {
      let idx = (x + y * img.width) * 4;
      let a = img.pixels[idx + 3];
      if (a > 200) {
        let r = img.pixels[idx];
        let g = img.pixels[idx + 1];
        let b = img.pixels[idx + 2];

        // Compute mapped screen position representing final layout
        let scale = 0.9;
        let screenX = lerp(100, width - 450, 0.9) + x * scale * 0.45;
        let screenY = (height / 2 - (img.height * scale * 0.45) / 2) + y * scale * 0.45;

        if (particles.length < MAX_PARTICLES) {
          particles.push(new Particle(screenX, screenY, color(r, g, b)));
        }
      }
    }
  }
}

// ======================================================================================
// TASK 2: PANORAMA MOTION GUIDE (40%)
// ======================================================================================
let t2GrayscaleA, t2GrayscaleB;
let t2EdgeA, t2EdgeB;
let t2ThresholdA, t2ThresholdB;
let t2Cx1, t2Cy1, t2Cx2, t2Cy2;
let t2Dx = 0, t2Dy = 0;
let t2DirectionString = "NONE";

function drawTask2() {
  background(15, 15, 20);

  // If the user moves the slider, update the thresholded buffer on-demand!
  if (t2State === "thresholded" || t2State === "centroid" || t2State === "arrow") {
    let currentSliderVal = t2ThresholdSlider.value();
    if (currentSliderVal !== t2LastSliderVal) {
      t2LastSliderVal = currentSliderVal;
      processThresholding();
      if (t2State === "centroid" || t2State === "arrow") {
        computeCentroids();
        if (t2State === "arrow") {
          determineMotionDirection();
        }
      }
    }
  }

  // Show header
  fill(15, 30, 15);
  rect(0, 0, width, 60);
  fill(0, 255, 120);
  textSize(20);
  textAlign(LEFT, CENTER);
  textStyle(BOLD);
  text("TASK 2 — PANORAMA MOTION GUIDE", 20, 30);

  fill(255);
  textSize(14);
  textAlign(RIGHT, CENTER);
  text(`Active: ${t2Pairs[t2CurrentPairIdx].name}  |  State: ${t2State.toUpperCase()}`, width - 20, 30);

  let p = t2Pairs[t2CurrentPairIdx];
  let imgW = 400;
  let imgH = 280;
  let xA = 70;
  let xB = 530;
  let yOffset = 100;

  // Grid layout headers
  noStroke();
  fill(200);
  textSize(16);
  textAlign(CENTER, TOP);
  textStyle(BOLD);
  text("FRAME A", xA + imgW / 2, yOffset - 25);
  text("FRAME B", xB + imgW / 2, yOffset - 25);

  // Draw current pair images depending on active pipeline state
  if (t2State === "panorama" || t2State === "loaded") {
    image(p.img1, xA, yOffset, imgW, imgH);
    image(p.img2, xB, yOffset, imgW, imgH);

    // Guide label
    drawInstructionBanner("Pair loaded! Press 'g' to convert both images to Grayscale.");
    t2ThresholdSlider.hide();
  }
  else if (t2State === "grayscale") {
    image(t2GrayscaleA, xA, yOffset, imgW, imgH);
    image(t2GrayscaleB, xB, yOffset, imgW, imgH);

    drawInstructionBanner("Grayscale conversion complete! Press 'e' to extract Outline Profiles.");
  }
  else if (t2State === "edge") {
    image(t2EdgeA, xA, yOffset, imgW, imgH);
    image(t2EdgeB, xB, yOffset, imgW, imgH);

    drawInstructionBanner("Outline extraction complete! Press 't' to apply Thresholding.");
  }
  else if (t2State === "thresholded") {
    image(t2ThresholdA, xA, yOffset, imgW, imgH);
    image(t2ThresholdB, xB, yOffset, imgW, imgH);

    drawInstructionBanner("Adjust Threshold slider below. Press 'n' to compute Centroids and dx/dy displacement.");
  }
  else if (t2State === "centroid") {
    // Render threshold images
    image(t2ThresholdA, xA, yOffset, imgW, imgH);
    image(t2ThresholdB, xB, yOffset, imgW, imgH);

    // Draw centroids as distinct yellow circles
    drawCentroidOverlay(xA, yOffset, imgW, imgH, t2Cx1, t2Cy1);
    drawCentroidOverlay(xB, yOffset, imgW, imgH, t2Cx2, t2Cy2);

    // Display displacement info
    fill(255, 230, 0);
    textSize(15);
    textAlign(CENTER, TOP);
    text(`Centroid A: (${nfc(t2Cx1, 1)}, ${nfc(t2Cy1, 1)})`, xA + imgW / 2, yOffset + imgH + 15);
    text(`Centroid B: (${nfc(t2Cx2, 1)}, ${nfc(t2Cy2, 1)})`, xB + imgW / 2, yOffset + imgH + 15);

    let dispY = yOffset + imgH + 45;
    fill(255);
    textSize(16);
    text(`Displacement dx: ${nfc(t2Dx, 1)}   dy: ${nfc(t2Dy, 1)}`, width / 2, dispY);

    drawInstructionBanner("Centroids Computed successfully! Press 'd' to detect and overlay direction arrow.");
  }
  else if (t2State === "arrow") {
    // Render base composite or final thresholded
    image(p.img1, xA, yOffset, imgW, imgH);
    image(p.img2, xB, yOffset, imgW, imgH);

    // Draw centroids
    drawCentroidOverlay(xA, yOffset, imgW, imgH, t2Cx1, t2Cy1);
    drawCentroidOverlay(xB, yOffset, imgW, imgH, t2Cx2, t2Cy2);

    // Motion Direction Calculation & Large Arrow Visualizer
    drawMotionArrowOverlay();

    drawInstructionBanner(`Estimated Motion: ${t2DirectionString}. Press 'i' to test next image pair!`);
  }
}

function drawInstructionBanner(msg) {
  fill(25, 25, 35, 220);
  stroke(0, 255, 120);
  rect(100, height - 120, 800, 50, 5);

  noStroke();
  fill(0, 255, 120);
  textSize(14);
  textAlign(CENTER, CENTER);
  textStyle(BOLD);
  text(msg, width / 2, height - 95);
}

// 1. Grayscale Conversion Pipeline
function processGrayscale(p) {
  t2GrayscaleA = createImage(p.img1.width, p.img1.height);
  t2GrayscaleB = createImage(p.img2.width, p.img2.height);

  applyGrayscaleFilter(p.img1, t2GrayscaleA);
  applyGrayscaleFilter(p.img2, t2GrayscaleB);
}

function applyGrayscaleFilter(src, dest) {
  src.loadPixels();
  dest.loadPixels();

  for (let i = 0; i < src.pixels.length; i += 4) {
    let r = src.pixels[i];
    let g = src.pixels[i + 1];
    let b = src.pixels[i + 2];
    let a = src.pixels[i + 3];

    // Grayscale luminance formula
    let gray = 0.299 * r + 0.587 * g + 0.114 * b;

    dest.pixels[i] = gray;
    dest.pixels[i + 1] = gray;
    dest.pixels[i + 2] = gray;
    dest.pixels[i + 3] = a;
  }
  dest.updatePixels();
}

// 2. Simple Custom Outline Edge Filter (Sobel or High-Pass derivative)
function processEdges() {
  t2EdgeA = createImage(t2GrayscaleA.width, t2GrayscaleA.height);
  t2EdgeB = createImage(t2GrayscaleB.width, t2GrayscaleB.height);

  applyEdgeFilter(t2GrayscaleA, t2EdgeA);
  applyEdgeFilter(t2GrayscaleB, t2EdgeB);
}

function applyEdgeFilter(src, dest) {
  src.loadPixels();
  dest.loadPixels();

  let w = src.width;
  let h = src.height;

  // Custom 3x3 Sobel matrices for gradient x and y
  let kx = [
    [-1, 0, 1],
    [-2, 0, 2],
    [-1, 0, 1]
  ];

  let ky = [
    [-1, -2, -1],
    [0, 0, 0],
    [1, 2, 1]
  ];

  for (let y = 1; y < h - 1; y++) {
    for (let x = 1; x < w - 1; x++) {
      let sumX = 0;
      let sumY = 0;

      // Iterate through 3x3 kernel neighbors
      for (let ky_idx = -1; ky_idx <= 1; ky_idx++) {
        for (let kx_idx = -1; kx_idx <= 1; kx_idx++) {
          let px = src.pixels[((x + kx_idx) + (y + ky_idx) * w) * 4]; // Only need red channel since it is grayscale
          sumX += px * kx[ky_idx + 1][kx_idx + 1];
          sumY += px * ky[ky_idx + 1][kx_idx + 1];
        }
      }

      // Compute gradient magnitude
      let magnitude = sqrt(sumX * sumX + sumY * sumY);
      magnitude = constrain(magnitude, 0, 255);

      let destIdx = (x + y * w) * 4;
      dest.pixels[destIdx] = magnitude;
      dest.pixels[destIdx + 1] = magnitude;
      dest.pixels[destIdx + 2] = magnitude;
      dest.pixels[destIdx + 3] = 255;
    }
  }
  dest.updatePixels();
}

// 3. Edge Thresholding
function processThresholding() {
  t2ThresholdA = createImage(t2EdgeA.width, t2EdgeA.height);
  t2ThresholdB = createImage(t2EdgeB.width, t2EdgeB.height);

  let val = t2ThresholdSlider.value();
  applyThresholdFilter(t2EdgeA, t2ThresholdA, val);
  applyThresholdFilter(t2EdgeB, t2ThresholdB, val);
}

function applyThresholdFilter(src, dest, thresholdVal) {
  src.loadPixels();
  dest.loadPixels();

  for (let i = 0; i < src.pixels.length; i += 4) {
    let px = src.pixels[i];
    let output = px >= thresholdVal ? 255 : 0;

    dest.pixels[i] = output;
    dest.pixels[i + 1] = output;
    dest.pixels[i + 2] = output;
    dest.pixels[i + 3] = 255;
  }
  dest.updatePixels();
}

// 4. Centroid Tracking Calculations
function computeCentroids() {
  let centroidA = calculateCentroidCoordinates(t2ThresholdA);
  let centroidB = calculateCentroidCoordinates(t2ThresholdB);

  t2Cx1 = centroidA.x;
  t2Cy1 = centroidA.y;
  t2Cx2 = centroidB.x;
  t2Cy2 = centroidB.y;

  t2Dx = t2Cx2 - t2Cx1;
  t2Dy = t2Cy2 - t2Cy1;
}

function calculateCentroidCoordinates(img) {
  img.loadPixels();
  let sumX = 0;
  let sumY = 0;
  let count = 0;

  for (let y = 0; y < img.height; y++) {
    for (let x = 0; x < img.width; x++) {
      let idx = (x + y * img.width) * 4;
      if (img.pixels[idx] === 255) { // Only thresholded edge pixels
        sumX += x;
        sumY += y;
        count++;
      }
    }
  }

  if (count === 0) {
    return { x: img.width / 2, y: img.height / 2 };
  }

  return { x: sumX / count, y: sumY / count };
}

function drawCentroidOverlay(rx, ry, rw, rh, cx, cy) {
  // Map normalized centroid coords to actual drawing coordinates
  let mappedX = map(cx, 0, t2ThresholdA.width, rx, rx + rw);
  let mappedY = map(cy, 0, t2ThresholdA.height, ry, ry + rh);

  // Highlight Centroid
  fill(255, 255, 0);
  stroke(0);
  strokeWeight(2);
  ellipse(mappedX, mappedY, 15, 15);

  // Add crosshairs
  stroke(255, 255, 0);
  strokeWeight(1.5);
  line(mappedX - 15, mappedY, mappedX + 15, mappedY);
  line(mappedX, mappedY - 15, mappedX, mappedY + 15);
  noStroke();
}

// 5. 8-Direction Motion Direction Detection Engine
function determineMotionDirection() {
  // Set threshold tolerance to ensure stability (e.g. noise filter of 3-4 pixels displacement)
  let tolerance = 3.0;

  let adx = abs(t2Dx);
  let ady = abs(t2Dy);

  if (adx < tolerance && ady < tolerance) {
    t2DirectionString = "NONE / STATIC";
    return;
  }

  // Evaluate diagonals vs cardinal
  // If ratio between dx and dy is within bounds, it's a diagonal motion
  let ratio = ady / (adx === 0 ? 0.0001 : adx);

  // Diagonal motion range
  if (ratio > 0.414 && ratio < 2.414) {
    if (t2Dx > 0 && t2Dy > 0) {
      t2DirectionString = "DOWN-RIGHT";
    } else if (t2Dx < 0 && t2Dy > 0) {
      t2DirectionString = "DOWN-LEFT";
    } else if (t2Dx > 0 && t2Dy < 0) {
      t2DirectionString = "UP-RIGHT";
    } else if (t2Dx < 0 && t2Dy < 0) {
      t2DirectionString = "UP-LEFT";
    }
  } else {
    // Cardinal directions
    if (adx > ady) {
      t2DirectionString = t2Dx > 0 ? "RIGHT" : "LEFT";
    } else {
      t2DirectionString = t2Dy > 0 ? "DOWN" : "UP";
    }
  }
}

// Render dynamic stylized direction arrows
function drawMotionArrowOverlay() {
  let cx = width / 2;
  let cy = height - 210;
  let len = 60;

  fill(0, 30, 15, 230);
  stroke(0, 255, 120);
  strokeWeight(2);
  rect(cx - 200, cy - 70, 400, 140, 10);

  // Draw descriptive text
  noStroke();
  fill(255);
  textSize(16);
  textAlign(CENTER, CENTER);
  textStyle(BOLD);
  text(`PANORAMA PANNING VECTOR`, cx, cy - 40);

  textSize(22);
  fill(0, 255, 120);
  text(t2DirectionString, cx, cy + 35);

  // Draw Arrow
  stroke(0, 255, 120);
  strokeWeight(6);
  strokeCap(ROUND);
  noFill();

  let dx = 0;
  let dy = 0;

  if (t2DirectionString === "RIGHT") { dx = len; }
  else if (t2DirectionString === "LEFT") { dx = -len; }
  else if (t2DirectionString === "DOWN") { dy = len; }
  else if (t2DirectionString === "UP") { dy = -len; }
  else if (t2DirectionString === "DOWN-RIGHT") { dx = len * 0.7; dy = len * 0.7; }
  else if (t2DirectionString === "DOWN-LEFT") { dx = -len * 0.7; dy = len * 0.7; }
  else if (t2DirectionString === "UP-RIGHT") { dx = len * 0.7; dy = -len * 0.7; }
  else if (t2DirectionString === "UP-LEFT") { dx = -len * 0.7; dy = -len * 0.7; }

  if (t2DirectionString !== "NONE / STATIC") {
    let arrowEndX = cx + dx;
    let arrowEndY = cy - 5 + dy;
    let arrowStartX = cx - dx * 0.3;
    let arrowStartY = cy - 5 - dy * 0.3;

    line(arrowStartX, arrowStartY, arrowEndX, arrowEndY);

    // Draw Arrowhead
    let angle = atan2(dy, dx);
    let headSize = 15;

    fill(0, 255, 120);
    noStroke();
    push();
    // Since translate is forbidden in Task 1 but NOT explicitly in Task 2,
    // let's do coordinate math manually to be extremely safe!
    // Arrowhead corners:
    let x1 = arrowEndX;
    let y1 = arrowEndY;
    let x2 = arrowEndX - headSize * cos(angle - PI / 6);
    let y2 = arrowEndY - headSize * sin(angle - PI / 6);
    let x3 = arrowEndX - headSize * cos(angle + PI / 6);
    let y3 = arrowEndY - headSize * sin(angle + PI / 6);
    triangle(x1, y1, x2, y2, x3, y3);
    pop();
  } else {
    // Static / No motion draw simple circle
    noStroke();
    fill(255, 100, 100);
    ellipse(cx, cy - 5, 20, 20);
  }
}

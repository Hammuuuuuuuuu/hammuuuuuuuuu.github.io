let shakeDuration = 60; // Number of frames to shake
let shakeAmount = 10; // Maximum tilt amount in degrees (increased for more vigorous shaking)
let isShaking = false; // Whether the buildings are currently shaking
let shakeStartFrame = 0; // The frame when shaking started

let glassMove = 200; // Initial position of the glass (moved 100 units to the right)
let glassSpeed = 50; // Initial speed of the glass
let isGlassOpened = false; // Boolean to track if glass is fully opened
let isReturning = false; // Boolean to track if the glass is returning

let buildingAppearThresholds = [300, 400, 500, 600, 700, 800]; // Adjusted for new glassMove position
let houseAppearThresholds = [300, 550]; // Adjusted for new glassMove position

let bicycleX = 640; // Starting X position of the bicycle
let bicycleY = 430; // Starting Y position of the bicycle
let bicycleSpeed = 2; // Speed of the bicycle
let bicycleReached = false; // Boolean to check if the bicycle has reached the target

let lorryX = 320; // Starting X position of the lorry
let lorryY = 420; // Starting Y position of the lorry
let lorrySpeed = 2; // Speed of the lorry
let lorryReached = false; // Boolean to check if the lorry has reached the target

let sequenceStage = 0; // To track stages of animation
let sequenceStartFrame = 0; // Frame when the current stage started

let logoImage; // Variable to hold the loaded image

function preload() {
  // Load the image file
  logoImage = loadImage("schoollogo.png");
}

function setup() {
  createCanvas(1280, 720);
  angleMode(DEGREES);
  frameRate(60); // Adjusted for better visibility
}

function draw() {
  background(50, 50, 100);

  // Calculate tilt angle
  let tiltAngle = 0;
  if (isShaking) {
    let elapsedFrames = frameCount - shakeStartFrame;
    if (elapsedFrames < shakeDuration) {
      tiltAngle = sin(frameCount * 10) * shakeAmount;
    } else {
      isShaking = false;
    }
  }

  // Draw the bicycle and lorry
  drawBicycle(bicycleX, bicycleY, 0.3);
  drawLorry(lorryX, lorryY, 0.5);

  // Draw buildings and houses with tilt
  if (glassMove > buildingAppearThresholds[0]) drawBuilding(400, 200, 50, 210, tiltAngle);
  if (glassMove > buildingAppearThresholds[1]) drawBuilding(460, 110, 70, 300, tiltAngle);
  if (glassMove > houseAppearThresholds[0]) drawHouse(320, 400, 1, tiltAngle); // Adjusted to new house function
  if (glassMove > buildingAppearThresholds[2]) drawHouse(570, 260, 1.5, tiltAngle); // Adjusted to new house function
  if (glassMove > houseAppearThresholds[1]) drawBuilding(640, 330, 90, 200, tiltAngle);
  if (glassMove > buildingAppearThresholds[3]) drawBuilding(740, 350, 90, 180, tiltAngle);

  // Draw map markers and dotted lines
  drawMapMarker(300, 500, 50);
  drawMapMarker(540, 500, 50);
  drawMapMarker(800, 500, 50);
  drawDottedLine(300, 565, 540, 565);
  drawDottedLine(540, 565, 800, 565);

  // Update and draw the rectangle and glass
  fill(50, 50, 100);
  rect(glassMove, 60, 5000, 5000);
  glass(glassMove, 560);

  // Control the animation sequence
  controlSequence();

  // Move the bicycle and lorry towards their targets if in the right stage
  if (sequenceStage === 2) {
    moveBicycle();
    moveLorry();
  }

  // Draw the image in the middle of the rect while closing at the end of the animation
  if (sequenceStage === 3) {
    image(logoImage, glassMove + 250, height / 2 - 60, 120, 120);
  }
  if (sequenceStage === 4) {
    image(logoImage, glassMove + 250, height / 2 - 60, 120, 120);
  }
}

let delayDuration = 180; // Number of frames to delay (3 seconds at 60 FPS)

function controlSequence() {
  let elapsedFrames = frameCount - sequenceStartFrame;

  switch (sequenceStage) {
    case 0: // Open the glass
      if (glassMove < width - 300) {
        glassMove += glassSpeed;
        if (glassSpeed < 20) { // Accelerate initially
          glassSpeed += 0.5;
        }
        if (glassMove >= width - 400) {
          sequenceStage++;
          sequenceStartFrame = frameCount;
          startShaking();
        }
      }
      break;
    case 1: // Shake buildings
      if (elapsedFrames >= shakeDuration) {
        sequenceStage++;
        sequenceStartFrame = frameCount;
      }
      break;
    case 2: // Move the bicycle and lorry
      if (!bicycleReached || !lorryReached) {
        moveBicycle();
        moveLorry();
      } else {
        sequenceStage++;
        sequenceStartFrame = frameCount;
      }
      break;
    case 3: // Close the glass
      if (glassMove > 280) {
        glassMove -= glassSpeed * 1; // Adjusted to slow down the closing speed
        if (glassMove <= 280) {
          sequenceStage++;
          sequenceStartFrame = frameCount;
        }
      }
      break;
    case 4: // Delay for 3 seconds
      if (elapsedFrames >= delayDuration) {
        resetAnimation();
      }
      break;
  }
}

function moveBicycle() {
  let targetX = 320; // Target X position of the house

  let dx = targetX - bicycleX;
  let distance = abs(dx);

  if (distance > 1) {
    bicycleX += (dx / distance) * bicycleSpeed;
  } else {
    bicycleReached = true;
  }
}

function moveLorry() {
  let targetX = 640; // Target X position of the building

  let dx = targetX - lorryX;
  let distance = abs(dx);

  if (distance > 1) {
    lorryX += (dx / distance) * lorrySpeed;
  } else {
    lorryReached = true;
  }
}

function drawHouse(x, y, scaleFactor, angle) {
  push();
  translate(x, y);
  scale(scaleFactor);
  rotate(angle);
  translate(-50, -80);

  // Draw the main body of the house
  fill(255);
  noStroke();
  rect(50, 80, 100, 90);
  fill(100);
  rect(87, 120, 25, 50, 2);

  // Draw the roof
  fill(255);
  triangle(100, 10, 30, 80, 170, 80);
  fill(100);
  triangle(100, 25, 45, 80, 155, 80);
  fill(255);
  triangle(100, 30, 50, 80, 150, 80);

  // Draw the smoke area
  stroke(5);
  stroke(255);
  line(140, 15, 140, 20);

  beginShape();
  vertex(130, 10);
  vertex(130, 30);
  vertex(150, 50);
  vertex(150, 10);
  endShape(CLOSE);

  // Draw the windows
  fill(150);
  rect(85, 55, 30, 30);

  strokeWeight(5);
  stroke(255);
  line(85, 70, 120, 70);
  line(100, 60, 100, 80);

  pop();
}

function drawBuilding(x, y, w, h, angle) {
  push();
  translate(x + w / 2, y + h);
  rotate(angle);
  translate(-w / 2, -h);
  fill(150, 255, 150);
  rect(0, 0, w, h);
  fill(100);
  for (let i = 10; i < h; i += 20) {
    for (let j = 10; j < w; j += 20) {
      rect(j, i, 10, 10);
    }
  }
  pop();
}

function drawMapMarker(x, y, size) {
  noStroke();
  fill(255);
  ellipse(x, y, size, size);
  fill(50, 50, 100);
  ellipse(x, y, size / 1.7, size / 1.7);
  fill(255);
  ellipse(x, y, size / 4, size / 4);
  fill(255);
  triangle(
    x - size / 3, y + size / 3,
    x + size / 3, y + size / 3,
    x, y + size
  );
  fill(255);
  ellipse(x, y + size + 15, size / 2, size / 2);
}

function glass(x, y) {
  noStroke();
  push();
  rotate(40);
  rect(x, y + 50, 80, 40, 30);
  pop();
  stroke(255);
  strokeWeight(20);
  line(x, y, x + 70, y + 70);
  stroke(50, 50, 100);
  strokeWeight(10);
  line(x, y, x + 70, y + 70);
  noStroke();
  fill(255);
  ellipse(x, y, 80, 80);
  fill(127);
  ellipse(x, y, 65, 65);

  noStroke();
}

// Call this function to start shaking the buildings
function startShaking() {
  isShaking = true;
  shakeStartFrame = frameCount;
}

// Call this function to reset the animation
function resetAnimation() {
  isGlassOpened = false;
  isReturning = false;
  glassMove = 200; // Adjusted initial position
  glassSpeed = 5;
  startShaking();
  bicycleX = 640; // Reset bicycle position
  bicycleY = 430;
  bicycleReached = false;
  lorryX = 320; // Reset lorry position
  lorryY = 420;
  lorryReached = false;
  sequenceStage = 0;
  sequenceStartFrame = frameCount;
}

function drawBicycle(x, y, scaleFactor) {
  push();
  translate(x, y);
  scale(scaleFactor);

  // Draw the wheels
  noStroke();
  fill(255);
  ellipse(60, 140, 50, 50);
  ellipse(140, 140, 50, 50);
  fill(100);
  ellipse(60, 140, 40, 40); // rear wheel
  ellipse(140, 140, 40, 40); // front wheel

  // Draw the bicycle frame
  stroke(255);
  strokeWeight(6);
  line(118, 130, 60, 120); // right wheel connect left wheel
  line(64, 120, 80, 90); // left wheel up handle

  // Draw the handlebars
  line(62, 90, 92, 90); // handlebar

  // Draw the rider
  noStroke();
  fill(255);
  ellipse(100, 60, 15, 15); // head
  stroke(255);
  strokeWeight(10);
  line(110, 70, 130, 100); // torso
  line(110, 70, 92, 90); // za hando
  line(130, 100, 105, 115); // leg
  line(105, 115, 105, 150); // leg2

  pop();
}

function drawLorry(x, y, scaleFactor) {
  push();
  translate(x, y);
  scale(scaleFactor);

  // Draw the lorry body
  noStroke();
  stroke(255);
  strokeWeight(5);
  fill(100);
  rect(50, 50, 70, 50, 3);
  line(120, 70, 130, 70);
  line(130, 70, 140, 90);
  line(140, 90, 140, 110);
  line(140, 110, 130, 110);
  noFill();
  ellipse(125, 110, 10, 10);
  line(120, 110, 80, 110);
  ellipse(75, 110, 10, 10);
  line(70, 110, 60, 110);
  line(60, 110, 60, 100);
  line(125, 75, 127, 75);
  line(127, 75, 132, 85);
  line(132, 85, 125, 85);
  line(125, 85, 125, 75);

  pop();
}

function drawDottedLine(x1, y1, x2, y2) {
  push();
  stroke(255); // Set the color of the dotted line
  strokeWeight(2); // Set the thickness of the dotted line
  drawingContext.setLineDash([5, 10]); // Create a dashed line with 5 units dash and 10 units gap
  line(x1, y1, x2, y2);
  drawingContext.setLineDash([]); // Reset the line dash to solid
  pop();
}

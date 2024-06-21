let shakeDuration = 60;
let shakeAmount = 10;
let isShaking = false;
let shakeStartFrame = 0;

let glassMove = 200;
let glassSpeed = 50;
let isGlassOpened = false;
let isReturning = false;

let buildingAppearThresholds = [300, 400, 500, 600, 700, 800];
let houseAppearThresholds = [300, 550];

let bicycleX = 640;
let bicycleY = 400;
let bicycleSpeed = 2;
let bicycleReached = false;

let lorryX = 320;
let lorryY = 390;
let lorrySpeed = 2;
let lorryReached = false;

let sequenceStage = 0;
let sequenceStartFrame = 0;

let logoImage;

function preload() {
logoImage = loadImage("schoollogo.png");
}

function setup() {
createCanvas(1280, 720);
angleMode(DEGREES);
frameRate(60);
}

function draw() {
background(50, 50, 100);

let tiltAngle = 0;
if (isShaking) {
let elapsedFrames = frameCount - shakeStartFrame;
if (elapsedFrames < shakeDuration) {
tiltAngle = sin(frameCount * 10) * shakeAmount;
} else {
isShaking = false;
}
}

drawBicycle(bicycleX, bicycleY, 0.4);
drawLorry(lorryX, lorryY, 0.6);

if (glassMove > buildingAppearThresholds[0]) drawBuilding(400, 200, 50, 210, tiltAngle);
if (glassMove > buildingAppearThresholds[1]) drawBuilding(460, 110, 70, 300, tiltAngle);
if (glassMove > houseAppearThresholds[0]) drawHouse(320, 400, 1, tiltAngle);
if (glassMove > buildingAppearThresholds[2]) drawHouse(570, 260, 1.5, tiltAngle);
if (glassMove > houseAppearThresholds[1]) drawBuilding(640, 330, 90, 200, tiltAngle);
if (glassMove > buildingAppearThresholds[3]) drawBuilding(740, 350, 90, 180, tiltAngle);

drawMapMarker(300, 500, 50);
drawMapMarker(540, 500, 50);
drawMapMarker(800, 500, 50);
drawDottedLine(300, 565, 540, 565);
drawDottedLine(540, 565, 800, 565);

fill(50, 50, 100);
rect(glassMove, 60, 5000, 5000);
glass(glassMove, 560);

controlSequence();

if (sequenceStage === 2) {
moveBicycle();
moveLorry();
}

if (sequenceStage === 3) {
image(logoImage, glassMove + 250, height / 2 - 60, 120, 120);
}
if (sequenceStage === 4) {
image(logoImage, glassMove + 250, height / 2 - 60, 120, 120);
}
}

let delayDuration = 180;

function controlSequence() {
let elapsedFrames = frameCount - sequenceStartFrame;

switch (sequenceStage) {
case 0:
if (glassMove < width - 300) {
glassMove += glassSpeed;
if (glassSpeed < 20) {
glassSpeed += 0.5;
}
if (glassMove >= width - 400) {
sequenceStage++;
sequenceStartFrame = frameCount;
startShaking();
}
}
break;
case 1:
if (elapsedFrames >= shakeDuration) {
sequenceStage++;
sequenceStartFrame = frameCount;
}
break;
case 2:
if (!bicycleReached || !lorryReached) {
moveBicycle();
moveLorry();
} else {
sequenceStage++;
sequenceStartFrame = frameCount;
}
break;
case 3:
if (glassMove > 280) {
glassMove -= glassSpeed * 1;
if (glassMove <= 280) {
sequenceStage++;
sequenceStartFrame = frameCount;
}
}
break;
case 4:
if (elapsedFrames >= delayDuration) {
resetAnimation();
}
break;
}
}

function moveBicycle() {
let targetX = 320;

let dx = targetX - bicycleX;
let distance = abs(dx);

if (distance > 1) {
bicycleX += (dx / distance) * bicycleSpeed;
} else {
bicycleReached = true;
}
}

function moveLorry() {
let targetX = 640;

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

fill(255);
noStroke();
rect(50, 80, 100, 90);
fill(100);
rect(87, 120, 25, 50, 2);

fill(255);
triangle(100, 10, 30, 80, 170, 80);
fill(100);
triangle(100, 25, 45, 80, 155, 80);
fill(255);
triangle(100, 30, 50, 80, 150, 80);

stroke(5);
stroke(255);
line(140, 15, 140, 20);

beginShape();
vertex(130, 10);
vertex(130, 30);
vertex(150, 50);
vertex(150, 10);
endShape(CLOSE);

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

function startShaking() {
isShaking = true;
shakeStartFrame = frameCount;
}

function resetAnimation() {
isGlassOpened = false;
isReturning = false;
glassMove = 200;
glassSpeed = 5;
startShaking();
bicycleX = 640;
bicycleY = 400;
bicycleReached = false;
lorryX = 320;
lorryY = 390;
lorryReached = false;
sequenceStage = 0;
sequenceStartFrame = frameCount;
}

function drawBicycle(x, y, scaleFactor) {
push();
translate(x, y);
scale(scaleFactor);

noStroke();
fill(255);
ellipse(60, 140, 50, 50);
ellipse(140, 140, 50, 50);
fill(100);
ellipse(60, 140, 40, 40);
ellipse(140, 140, 40, 40);

stroke(255);
strokeWeight(6);
line(118, 130, 60, 120);
line(64, 120, 80, 90);

line(62, 90, 92, 90);

noStroke();
fill(255);
ellipse(100, 60, 15, 15);
stroke(255);
strokeWeight(10);
line(110, 70, 130, 100);
line(110, 70, 92, 90);
line(130, 100, 105, 115);
line(105, 115, 105, 150);

pop();
}

function drawLorry(x, y, scaleFactor) {
push();
translate(x, y);
scale(scaleFactor);

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
stroke(255);
strokeWeight(2);
drawingContext.setLineDash([5, 10]);
line(x1, y1, x2, y2);
drawingContext.setLineDash([]);
pop();
} 

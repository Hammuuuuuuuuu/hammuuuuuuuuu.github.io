let player;
let obstacles = [];
let gravity = 0.6;
let jumpStrength = 15;
let obstacleSpeed = 5;
let score = 0;
let gameOver = false;
let startButton, restartButton, bgMusic;
let gameStarted = false;
let backgroundImage;
let blockTimer = 0;
let spikeTimer = 0;
let blockInterval = 120;
let spikeInterval = 180;
let minObstacleDistance = 100;
let backgroundX = 0;
let backgroundSpeed = 2;
let selectedColor = 'red';
let selectedDesign = 'square';
let colorButtons = [];
let designButtons = [];
let songButtons = [];
let song1, song2, song3;
let currentSong;

function preload() {
  bgMusic = loadSound('unwelcome.mp3');
  song1 = loadSound('swaytomybeat.mp3');
  song2 = loadSound('stopheartbreaking.mp3');
  song3 = loadSound('thingwithfeather.mp3');
  backgroundImage = loadImage('gamebg.jpg');
}

function setup() {
  createCanvas(1280, 700);
  player = new Player();

  startButton = createButton('Start');
  startButton.position(width / 2 - 50, height / 2 + 50);
  startButton.size(100, 50);
  startButton.style('border', 'none');
  startButton.style('font-size', '24px');
  startButton.style('font-family', 'Arial');
  startButton.style('background-color', '#FF0000');
  startButton.style('color', '#FFFFFF');
  startButton.mousePressed(startGame);

  restartButton = createButton('Restart');
  restartButton.position(width / 2 - 50, height / 2);
  restartButton.size(100, 50);
  restartButton.style('font-size', '24px');
  restartButton.style('font-family', 'Arial');
  restartButton.style('background-color', '#FF0000');
  restartButton.style('color', '#FFFFFF');
  restartButton.mousePressed(restartGame);
  restartButton.hide();

  createColorButtons();
  createDesignButtons();
  createSongButtons();
}

function draw() {
  if (!gameStarted) {
    drawStartScreen();
  } else {
    background(220);
    image(backgroundImage, backgroundX, 0, width, height);
    image(backgroundImage, backgroundX + width, 0, width, height);

    backgroundX -= backgroundSpeed;
    if (backgroundX <= -width) {
      backgroundX = 0;
    }

    if (!gameOver) {
      player.update();
      player.show();

      blockTimer++;
      spikeTimer++;

      if (blockTimer >= blockInterval) {
        if (canSpawnObstacle()) {
          obstacles.push(new Obstacle('block'));
          blockTimer = 0;
          score++;
          increaseSpeed();
        }
      }
      if (spikeTimer >= spikeInterval) {
        if (canSpawnObstacle()) {
          obstacles.push(new Obstacle('spike'));
          spikeTimer = 0;
          score++;
          increaseSpeed();
        }
      }

      for (let i = obstacles.length - 1; i >= 0; i--) {
        obstacles[i].update();
        obstacles[i].show();

        if (obstacles[i].hits(player)) {
          gameOver = true;
          noLoop();
          restartButton.show();
          stopAllMusic();
        }

        if (obstacles[i].offscreen()) {
          obstacles.splice(i, 1);
        }
      }

      fill(0);
      textSize(32);
      textAlign(LEFT, TOP);
      text('Score: ' + score, 10, 10);
    } else {
      fill(0);
      textSize(32);
      textAlign(LEFT, TOP);
      text('Score: ' + score, 10, 10);
    }
  }
}

function drawStartScreen() {
  background(220);
  image(backgroundImage, 0, 0, width, height);
  drawText();
  drawPlayerCharacter();
}

function startGame() {
  gameStarted = true;
  startButton.hide();
  colorButtons.forEach(button => button.hide());
  designButtons.forEach(button => button.hide());
  songButtons.forEach(button => button.hide());
}

class Player {
  constructor() {
    this.x = 50;
    this.y = height - 50;
    this.size = 50;
    this.velocity = 0;
    this.angle = 0;
  }

  update() {
    this.velocity += gravity;
    this.y += this.velocity;

    if (this.y >= height - this.size) {
      this.y = height - this.size;
      this.velocity = 0;
      this.angle = 0;
    } else {
      this.angle += 15;
    }
  }

  show() {
    fill(selectedColor);
    push();
    translate(this.x + this.size / 2, this.y + this.size / 2);
    rotate(radians(this.angle));
    rectMode(CENTER);
    if (selectedDesign === 'square') {
      rect(0, 0, this.size, this.size);
    } else if (selectedDesign === 'circle') {
      ellipse(0, 0, this.size, this.size);
    }
    pop();
  }

  jump() {
    if (this.y === height - this.size) {
      this.velocity = -jumpStrength;
    }
  }
}

class Obstacle {
  constructor(type) {
    this.x = width;
    this.size = 50;
    this.type = type;
    if (this.type === 'block') {
      this.y = height - this.size + 25; // Ensure block touches the ground
    } else if (this.type === 'spike') {
      this.y = height; // Ensure spike touches the ground
    }
  }

  update() {
    this.x -= obstacleSpeed;
  }

  show() {
    fill(0);
    if (this.type === 'block') {
      rect(this.x, this.y, this.size, this.size);
    } else if (this.type === 'spike') {
      triangle(this.x, this.y, this.x + this.size / 2, this.y - this.size, this.x + this.size, this.y);
    }
  }

  hits(player) {
    if (this.type === 'block') {
      return (
        player.x + player.size > this.x &&
        player.x < this.x + this.size &&
        player.y + player.size > this.y
      );
    } else if (this.type === 'spike') {
      let spikeX1 = this.x;
      let spikeX2 = this.x + this.size / 2;
      let spikeX3 = this.x + this.size;
      let spikeY1 = this.y;
      let spikeY2 = this.y - this.size;
      let spikeY3 = this.y;

      return (
        (spikeX1 >= player.x && spikeX1 <= player.x + player.size && spikeY1 >= player.y && spikeY1 <= player.y + player.size) ||
        (spikeX2 >= player.x && spikeX2 <= player.x + player.size && spikeY2 >= player.y && spikeY2 <= player.y + player.size) ||
        (spikeX3 >= player.x && spikeX3 <= player.x + player.size && spikeY3 >= player.y && spikeY3 <= player.y + player.size)
      );
    }
  }

  offscreen() {
    return this.x < -this.size;
  }
}

function keyPressed() {
  if (key === ' ' && !gameOver && gameStarted) {
    player.jump();
  }
}

function restartGame() {
  player = new Player();
  obstacles = [];
  score = 0;
  gameOver = false;
  blockTimer = 0;
  spikeTimer = 0;
  jumpStrength = 15;
  obstacleSpeed = 5;

  restartButton.hide();
  loop();

  playCurrentSong();
}

function canSpawnObstacle() {
  if (obstacles.length > 0) {
    let lastObstacle = obstacles[obstacles.length - 1];
    if (width - lastObstacle.x < minObstacleDistance) {
      return false;
    }
  }
  return true;
}

function increaseSpeed() {
  if (score % 15 === 0) {
    obstacleSpeed += 1;
    jumpStrength += 2;
  }
}

function drawText() {
  noFill();
  noStroke();
  fill("RED");
  stroke(0);
  strokeWeight(1);
  beginShape();
  vertex(100, 100);
  vertex(130, 100);
  vertex(130, 120);
  vertex(115, 120);
  vertex(115, 160);
  vertex(120, 160);
  vertex(120, 140);
  vertex(130, 140);
  vertex(130, 180);
  vertex(100, 180);
  endShape(CLOSE);

  fill("ORANGE");
  rect(150, 110, 30, 20);
  rect(150, 140, 20, 20);
  rect(150, 170, 30, 20);

  fill("YELLOW");
  rect(200, 140, 30, 80);
  rect(200, 140, 20, 60);

  stroke("GREEN");
  strokeWeight(5);
  line(240, 180, 240, 100);
  line(240, 100, 265, 140);
  line(265, 140, 285, 100);
  line(285, 100, 285, 180);

  fill("BLUE");
  strokeWeight(1);
  stroke(0);
  rect(320, 110, 40, 20);
  rect(320, 140, 20, 20);
  rect(320, 170, 30, 20);

  fill("INDIGO");
  stroke("INDIGO");
  strokeWeight(1);
  rect(380, 110, 50, 20);
  rect(380, 140, 20, 80);

  fill("PURPLE");
  stroke("PURPLE");
  strokeWeight(10);
  line(420, 120, 470, 180);
  stroke(0);
  strokeWeight(1);
  rect(427, 150, 20, 70);
  rect(440, 120, 50, 40, 20);

  stroke("#BB6588");
  strokeWeight(10);
  line(500, 105, 525, 140);
  line(525, 140, 550, 105);
  line(525, 140, 525, 180);

  rect(395, 280, 30, 60, 3, 30, 30, 3);
  fill(220);
  rect(395, 280, 30, 60, 3, 30, 30, 3);

  line(460, 250, 440, 310);
  line(460, 250, 480, 310);
  line(450, 280, 470, 280);

  line(520, 250, 500, 280);
  line(500, 280, 540, 280);
  line(540, 280, 520, 310);

  line(570, 250, 570, 310);
  line(610, 250, 610, 310);
  line(570, 280, 610, 280);
  noStroke();
}

function createColorButtons() {
  const colors = ['red', 'green', 'blue', 'yellow', 'purple'];
  for (let i = 0; i < colors.length; i++) {
    let button = createButton(colors[i]);
    button.position(50 + i * 60, height - 50);
    button.style('background-color', colors[i]);
    button.mousePressed(() => selectColor(colors[i]));
    colorButtons.push(button);
  }
}

function createDesignButtons() {
  const designs = ['square', 'circle'];
  for (let i = 0; i < designs.length; i++) {
    let button = createButton(designs[i]);
    button.position(width - 150 + i * 60, height - 50);
    button.mousePressed(() => selectDesign(designs[i]));
    designButtons.push(button);
  }
}

function createSongButtons() {
  const songs = ['Sway to My Beat in Cosmos', 'If I Can Stop One Heart From Breaking', 'Hope is The Thing With Feathers'];
  const songFiles = [song1, song2, song3];
  for (let i = 0; i < songs.length; i++) {
    let button = createButton(songs[i]);
    button.position(width / 2 - 150 + i * 110, height / 2 + 150);
    button.size(100, 70);
    button.mousePressed(() => selectSong(songFiles[i]));
    songButtons.push(button);
  }
}

function selectColor(color) {
  selectedColor = color;
}

function selectDesign(design) {
  selectedDesign = design;
}

function selectSong(song) {
  if (currentSong) {
    currentSong.stop();
  }
  currentSong = song;
  currentSong.loop();
}

function playCurrentSong() {
  if (currentSong) {
    currentSong.loop();
  }
}

function stopAllMusic() {
  song1.stop();
  song2.stop();
  song3.stop();
}

function drawPlayerCharacter() {
  fill(selectedColor);
  rectMode(CENTER);
  if (selectedDesign === 'square') {
    rect(width / 2, 650, 50, 50);
    noStroke();

  } else if (selectedDesign === 'circle') {
    ellipse(width / 2, 650, 50, 50);

    noStroke();
  }
}

let firstSlider, secondSlider, thirdSlider, fourthSlider, fifthSlider;

function setup() {
  can = createCanvas(1200, 800);
  can.position(0, 20);
  firstSlider = createSlider(1, 37, 20);
  secondSlider = createSlider(0, 255, 50);
  thirdSlider = createSlider(0, 255, 50);
  fourthSlider = createSlider(0, 255, 50);
  fifthSlider = createSlider(0, 255, 50);
  angleMode(DEGREES);
}

function draw() {
  background(0);

  let first = firstSlider.value();
  let second = secondSlider.value();
  let third = thirdSlider.value();
  let fourth = fourthSlider.value();
  let fifth = fifthSlider.value();

  let color1 = color(fourth, 0, 0, second); 
  let color2 = color(0, fifth, 255, third); 

  stroke(255);
  strokeWeight(3);
  line(width / 2, height / 2, 600, 800);

  strokeWeight(1);
  for (let i = 0; i < first; i++) {
    push();
    translate(width / 2, height / 2);
    rotate(i * 360 / first);

    let position = map(i, 0, first, 0, 1);
    let lerpedColor = lerpColor(color1, color2, position);
    fill(lerpedColor);
    stroke(255);

    rect(40, 40, 100, 100);
    pop();
  }
}

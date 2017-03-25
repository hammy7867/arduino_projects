import processing.serial.*;
Serial myPort;
PrintWriter output;
String val;
String delim = "; ";

void setup() {
  String portName = Serial.list()[11];
  myPort = new Serial(this, portName, 9600);
  output = createWriter("data.txt");
}

void draw() {
  if ( myPort.available() > 0) {
    val = myPort.readStringUntil('\n');  // read it and store in in val
    if (val != null) {
      output.print(val);
    }
  }
}

void keyPressed() {
  output.flush(); // Writes remaining data to the file
  output.close(); // Finishes the file
  exit();
}
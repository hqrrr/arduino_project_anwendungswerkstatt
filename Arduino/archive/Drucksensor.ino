int val;
void setup() {
  // put your setup code here, to run once:
pinMode(A0, INPUT);
Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
val=analogRead(A0);
Serial.println(val);
//if val!=0 dann fangen Sensoren an zu messen
if (val!=0)
{
  Serial.println("Jemand sitzt, Messung beginnen!");
}
delay(2000);
}

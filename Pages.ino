//#########################################################################################
String CreateHomePage(String sitetitle, String version, String year){
  ReadSensors();                      // Get sensor readings, or get simulated values if 'simulated' is ON
  String webpage = "";
  webpage += CreateHtmlHeader(sitetitle, "home", true);
  webpage += "<div class=\"centre\">";
  webpage += "<div class=\"div-table\">";
  webpage += "<div class=\"div-table-row\">";
  webpage += "<div class=\"div-table-col\" style=\"width: " + String(settings.ManualOverride == 1 ? "14.2" : "16.6") + "%;\">Temperature</div>";
  webpage += "<div class=\"div-table-col\" style=\"width: " + String(settings.ManualOverride == 1 ? "14.2" : "16.6") + "%;\">Humidity</div>";
  webpage += "<div class=\"div-table-col\" style=\"width: " + String(settings.ManualOverride == 1 ? "14.2" : "16.6") + "%;\">Target Temperature</div>";
  webpage += "<div class=\"div-table-col\" style=\"width: " + String(settings.ManualOverride == 1 ? "14.2" : "16.6") + "%;\">Thermostat Status</div>";
  webpage += "<div class=\"div-table-col\" style=\"width: " + String(settings.ManualOverride == 1 ? "14.2" : "16.6") + "%;\">Schedule Status</div>";
  if (settings.ManualOverride == 1) {
    webpage += "<div class=\"div-table-col\" style=\"width: 14.2%;\">ManualOverride</div>";
  }
  webpage += "<div class=\"div-table-col\" style=\"width: " + String(settings.ManualOverride == 1 ? "14.2" : "16.6") + "%;\"><div class='wifi'/></div></div>";
  webpage += "</div>";
  webpage += "<div class=\"div-table-row\">";
  webpage += "<div class=\"div-table-col large\" style=\"width: " + String(settings.ManualOverride == 1 ? "14.2" : "16.6") + "%;\">" + String(Temperature, 1) + "&deg;</div>";
  webpage += "<div class=\"div-table-col large\" style=\"width: " + String(settings.ManualOverride == 1 ? "14.2" : "16.6") + "%;\">" + String(Humidity, 0) + "%</div>";
  webpage += "<div class=\"div-table-col large\" style=\"width: " + String(settings.ManualOverride == 1 ? "14.2" : "16.6") + "%;\">" + String(TargetTemp, 1) + "&deg;</div>";
  webpage += "<div class=\"div-table-col large\" style=\"width: " + String(settings.ManualOverride == 1 ? "14.2" : "16.6") + "%;\"><span class=" + String((RelayState == "ON" ? "'on'>" : "'off'>")) + RelayState + "</span></div>"; 
  webpage += "<div class=\"div-table-col large\" style=\"width: " + String(settings.ManualOverride == 1 ? "14.2" : "16.6") + "%;\"><span class=" + String((settings.TimerState == "ON" ? "'on'>" : "'off'>")) + settings.TimerState + "</span></div>";
  if (settings.ManualOverride == 1) {
    webpage += "<div class=\"div-table-col large\" style=\"width: 14.2%;\">" + String(settings.ManualOverride == 1 ? "ON" : "OFF") + "</div>";
  }
  webpage += "<div class=\"div-table-col large\" style=\"width: " + String(settings.ManualOverride == 1 ? "14.2" : "16.6") + "%;\"><span>" + WiFiSignal() + "</span></div>";
  webpage += "</div>";
  webpage += "</div>";
  webpage += "</div>";
  webpage += "<br>";
  webpage += "<h2>" + sitetitle + " Status</h2><br>";
  webpage += "<div class='numberCircle'><span class=" + String((RelayState == "ON" ? "'on'>" : "'off'>")) + String(Temperature, 1) + "&deg;</span></div><br><br><br>";
  webpage += CreateHtmlFooter(version, year);
  return webpage;
}
//#########################################################################################
String CreateGraphsPage(String sitetitle, String version, String year){
  String webpage = "";
  webpage += CreateHtmlHeader(sitetitle, "graphs", true);
  webpage += "<h2>" + sitetitle + " Readings</h2>";
  webpage += "<script type='text/javascript' src='https://www.gstatic.com/charts/loader.js'></script>";
  webpage += "<script type='text/javascript'>";
  webpage += "google.charts.load('current', {'packages':['corechart']});";
  webpage += "google.charts.setOnLoadCallback(drawGraphT);";  // Pre-load function names for Temperature graphs
  webpage += "google.charts.setOnLoadCallback(drawGraphH);";  // Pre-load function names for Humidity graphs
  webpage += AddGraph("GraphT", "Temperature", "TS", "°C", "red", "chart_div");
  webpage += AddGraph("GraphH", "Humidity", "HS", "%", "blue", "chart_div");
  webpage += "</script>";
  webpage += "<div id='outer'>";
  webpage += "<div class=\"div-table\">";
  webpage += "<div class=\"div-table-row\">";
  webpage += "  <div class=\"div-table-col\" style=\"width: 50%\"><div id='chart_divTS'></div></div>";
  webpage += "  <div class=\"div-table-col\" style=\"width: 50%\"><div id='chart_divHS'></div></div>";
  webpage += "</div>";
  webpage += "</div>";
  webpage += "<br>";
  webpage += "</div>";
  webpage += "<p>Heating status : <span class=" + String((RelayState == "ON" ? "'on'>" : "'off'>")) + RelayState + "</span></p>";
  webpage += CreateHtmlFooter(version, year);
  return webpage;
}
String PreLoadChartData(String Type) {
  byte r = 0;
  String Data = "";
  do {
    if (Type == "Temperature") {
      Data += "[" + String(r) + "," + String(sensordata[r].Temperature, 1) + "," + String(TargetTemp, 1) + "],";
    } else {
      Data += "[" + String(r) + "," + String(sensordata[r].Humidity) + "],";
    }
    r++;
  } while (r < SensorReadings);
  Data += "]";
  return Data;
}
String AddGraph(String Type, String Title, String GraphType, String Units, String Colour, String Div) {
  String webpage = "";
  const String legendColour = "black";     // Only use HTML colour names
  const String titleColour = "purple";
  const String backgrndColour = "transparent";
  const String data1Colour = "red";
  const String data2Colour = "orange";
  String Data = PreLoadChartData(Title);
  webpage += "function draw" + Type + "() {";
  if (Type == "GraphT") {
    webpage += " var data = google.visualization.arrayToDataTable(" + String("[['Hour', 'Rm T°', 'Tgt T°'],") + Data + ");";
  } else
    webpage += " var data = google.visualization.arrayToDataTable(" + String("[['Hour', 'RH %'],") + Data + ");";
  webpage += " var options = {";
  webpage += "  title: '" + Title + "',";
  webpage += "  titleFontSize: 14,";
  webpage += "  backgroundColor: '" + backgrndColour + "',";
  webpage += "  legendTextStyle: { color: '" + legendColour + "' },";
  webpage += "  titleTextStyle:  { color: '" + titleColour + "' },";
  webpage += "  hAxis: {color: '#FFF'},";
  webpage += "  vAxis: {color: '#FFF', title: '" + Units + "'},";
  webpage += "  curveType: 'function',";
  webpage += "  pointSize: 1,";
  webpage += "  lineWidth: 1,";
  webpage += "  width:  450,";
  webpage += "  height: 280,";
  webpage += "  colors:['" + Colour + (Type == "GraphT" ? "', 'orange" : "") + "'],";
  webpage += "  legend: { position: 'right' }";
  webpage += " };";
  webpage += " var chart = new google.visualization.LineChart(document.getElementById('" + Div + GraphType + "'));";
  webpage += "  chart.draw(data, options);";
  webpage += " };";
  return webpage;
}
//#########################################################################################
String CreateSchedulePage(String sitetitle, String version, String year){
  String webpage = "";
  webpage += CreateHtmlHeader(sitetitle, "timer", false);
  webpage += "<h2>Thermostat Schedule Setup</h2><br>";
  webpage += "<h3>Enter required temperatures and time, use Clock symbol for ease of time entry</h3><br>";
  webpage += "<form action='/dotimer' method='POST'>";
  webpage += "<div class=\"div-table\">";
  webpage += "<div class=\"div-table-row\">";
  webpage += "  <div class=\"div-table-col\" style=\"width: 12.5%\">Control</div>";
  webpage += "  <div class=\"div-table-col\" style=\"width: 12.5%\">" + Timer[0].DoW + "</div>";
  for (byte dow = 1; dow < 6; dow++) {  // Heading line showing DoW
    webpage += "  <div class=\"div-table-col\" style=\"width: 12.5%\">" + Timer[dow].DoW + "</div>";
  }
  webpage += "  <div class=\"div-table-col\" style=\"width: 12.5%\">" + Timer[6].DoW + "</div>";
  webpage += "</div>";
  for (byte p = 0; p < NumOfEvents; p++) {
    webpage += "<div class=\"div-table-row\">";
    webpage += "  <div class=\"div-table-col\" style=\"width: 12.5%\">Temp</div>";
    webpage += "  <div class=\"div-table-col\" style=\"width: 12.5%\"><input type='text' name='" + String(0) + "." + String(p) + ".Temp' value='" + Timer[0].Temp[p] + "' maxlength='5' size='6'></div>";
    for (int dow = 1; dow < 6; dow++) {
      webpage += "  <div class=\"div-table-col\" style=\"width: 12.5%\"><input type='text' name='" + String(dow) + "." + String(p) + ".Temp' value='" + Timer[dow].Temp[p] + "' maxlength='5' size='5'></div>";
    }
    webpage += "  <div class=\"div-table-col\" style=\"width: 12.5%\"><input type='text' name='" + String(6) + "." + String(p) + ".Temp' value='" + Timer[6].Temp[p] + "' maxlength='5' size='5'></div>";
    webpage += "</div>";
    webpage += "<div class=\"div-table-row\">";
    webpage += "  <div class=\"div-table-col\" style=\"width: 12.5%\">Start</div>";
    webpage += "  <div class=\"div-table-col\" style=\"width: 12.5%\"><input type='time' name='" + String(0) + "." + String(p) + ".Start' value='" + Timer[0].Start[p] + "'></div>";
    for (int dow = 1; dow < 6; dow++) {
      webpage += "  <div class=\"div-table-col\" style=\"width: 12.5%\"><input type='time' name='" + String(dow) + "." + String(p) + ".Start' value='" + Timer[dow].Start[p] + "'></div>";
    }
    webpage += "  <div class=\"div-table-col\" style=\"width: 12.5%\"><input type='time' name='" + String(6) + "." + String(p) + ".Start' value='" + Timer[6].Start[p] + "'></div>";
    webpage += "</div>";
    webpage += "<div class=\"div-table-row\">";
    webpage += "  <div class=\"div-table-col\" style=\"width: 12.5%\">Stop</div>";
    webpage += "  <div class=\"div-table-col\" style=\"width: 12.5%\"><input type='time' name='" + String(0) + "." + String(p) + ".Stop' value='" + Timer[0].Stop[p] + "'></div>";
    for (int dow = 1; dow < 6; dow++) {
      webpage += "  <div class=\"div-table-col\" style=\"width: 12.5%\"><input type='time' name='" + String(dow) + "." + String(p) + ".Stop' value='" + Timer[dow].Stop[p] + "'></div>";
    }
    webpage += "  <div class=\"div-table-col\" style=\"width: 12.5%\"><input type='time' name='" + String(6) + "." + String(p) + ".Stop' value='" + Timer[6].Stop[p] + "'></div>";
    webpage += "</div>";
    if (p < (NumOfEvents - 1)) {
      webpage += "<div class=\"div-table-row\"><div class=\"div-table-col\" style=\"width: 12.5%\">x</div><div class=\"div-table-col\" style=\"width: 12.5%\">x</div>";
      for (int dow = 2; dow < 7; dow++) {
        webpage += "<div class=\"div-table-col\" style=\"width: 12.5%\">-</div>";
      }
      webpage += "<div class=\"div-table-col\" style=\"width: 12.5%\">x</div></div>";
    }
  }
  webpage += "</div>";
  webpage += "<div class='centre'>";
  webpage += "<br><input type='submit' value='Enter'><br><br>";
  webpage += "</div>";
  webpage += "</form>";
  webpage += CreateHtmlFooter(version, year);
  return webpage;
}
//#########################################################################################
String CreateSetupPage(String sitetitle, String version, String year){
  String webpage = "";
  webpage += CreateHtmlHeader(sitetitle, "setup", false);
  webpage += "<h2>" + sitetitle + " Setup</h2><br>";
  webpage += "<h3>Enter required parameter values or reset to default by clicking <a href='/doreset' onclick=\"return window.confirm('?');\">here</a></h3><br>";
  webpage += "<form action='/dosetup' method=\"POST\">";
  webpage += "<div class=\"centre\">";
  webpage += "<div class=\"div-table\">";
  webpage += "<div class=\"div-table-row\">";
  webpage += "<div class=\"div-table-col\" style=\"width: 50%\">Setting</div><div class=\"div-table-col\" style=\"width: 50%\">Value</div>";
  webpage += "</div>";
  webpage += "<div class=\"div-table-row\">";
  webpage += "<div class=\"div-table-col\" style=\"width: 50%\"><label for='hysteresis'>Default temperature value (e.g. 0 - 28&deg;) [NN]</label></div>";
  webpage += "<div class=\"div-table-col\" style=\"width: 50%\"><input type='text' size='4' pattern='[0-9]*' name='defaulttemp' value='" + String(settings.TargetTemp, 0) + "' style=\"width: 60px\"></div>";  // 0.0 valid input style
  webpage += "</div>";
  webpage += "<div class=\"div-table-row\">";
  webpage += "<div class=\"div-table-col\" style=\"width: 50%\"><label for='hysteresis'>Hysteresis value (e.g. 0 - 1.0&deg;) [N.N]</label></div>";
  webpage += "<div class=\"div-table-col\" style=\"width: 50%\"><input type='text' size='4' pattern='[0-9][.][0-9]' name='hysteresis' value='" + String(settings.Hysteresis, 1) + "' style=\"width: 60px\"></div>";  // 0.0 valid input style
  webpage += "</div>";
  webpage += "<div class=\"div-table-row\">";
  webpage += "<div class=\"div-table-col\" style=\"width: 50%\"><label for='frosttemp'>Frost Protection Temperature&deg; [NN]</label></div>";
  webpage += "<div class=\"div-table-col\" style=\"width: 50%\"><input type='text' size='4' pattern='[0-9]*' name='frosttemp' value='" + String(settings.FrostTemp) + "' style=\"width: 60px\"></div>";  // 00-99 valid input style
  webpage += "</div>";
  webpage += "<div class=\"div-table-row\">";
  webpage += "<div class=\"div-table-col\" style=\"width: 50%\"><label for='earlystart'>Early start duration (mins) [NN]</label></div>";
  webpage += "<div class=\"div-table-col\" style=\"width: 50%\"><input type='text' size='4' pattern='[0-9]*' name='earlystart' value='" + String(settings.EarlyStart) + "' style=\"width: 60px\"></div>";  // 00-99 valid input style
  webpage += "</div>";
  webpage += "<div class=\"div-table-row\">";
  webpage += "<div class=\"div-table-col\" style=\"width: 50%\"><label for='manualoveride'>Manual heating override </label></div>";
  webpage += "<div class=\"div-table-col\" style=\"width: 50%\"><select name='manualoverride' style='width: 100px'>";
  webpage += "<option "+ String(settings.ManualOverride == 1 ? "selected" : "") +" value='1'>ON</option>";
  webpage += "<option "+ String(settings.ManualOverride == 0 ? "selected" : "") +" value='0'>OFF</option></select></div>";  // ON/OFF
  webpage += "</div>";
  webpage += "<div class=\"div-table-row\">";
  webpage += "<div class=\"div-table-col\" style=\"width: 50%\"><label for='manualoverridetemp'>Manual Override Temperature&deg; </label></div>";
  webpage += "<div class=\"div-table-col\" style=\"width: 50%\"><input type='text' size='4' pattern='[0-9]*' name='manualoverridetemp' value='" + String(settings.ManOverrideTemp, 0) + "' style=\"width: 60px\"></div>";  // 00-99 valid input style
  webpage += "</div>";
  webpage += "</div>";
  webpage += "</div>";
  webpage += "<br><input type='submit' value='Enter'><br><br>";
  webpage += "</form>";
  webpage += CreateHtmlFooter(version, year);
  return webpage;
}
//#########################################################################################
String CreateHelpPage(String sitetitle, String version, String year){
  String webpage = "";
  webpage += CreateHtmlHeader(sitetitle, "help", false);
  webpage += R"rawliteral(
<h2>Help</h2><br>
<div style='text-align: left;font-size:1.1em;'>
<br>
<u><b>Home</b></u>
<p>Displays the current temperature and humidity. 
Displays the targeted temperature, the current state of the thermostat (ON/OFF) and timer status (ON/OFF).</p>
<u><b>Graphs</b></u>
<p>Displays the target temperature set and the current measured temperature and humidity. 
Thermostat status is also displayed as temperature varies.</p>
<u><b>Schedules</b></u>
<p>Determines the heating temperature for each day of the week and up to 4 heating periods in a day. 
To set the heating to come on at 06:00 and off at 09:00 with a temperature of 20&deg; enter 20 then the required start/end times. 
Repeat for each day of the week and heating period within the day for the required heat profile.</p>
<u><b>Setup</b></u>
<p><i>Default Temperature</i> - used to set the targeted temperature.</p>
<p><i>Hysteresis</i> - this setting is used to prevent unwanted rapid switching on/off of the heating as the room temperature
 nears or falls towards the set/target-point temperature. A normal setting is 0.5&deg;C, the exact value depends on the environmental characteristics, 
for example, where the thermostat is located and how fast a room heats or cools.</p>
<p><i>Frost Protection Temperature</i> - this setting is used to protect from low temperatures and pipe freezing in cold conditions. 
It helps prevent low temperature damage by turning on the heating until the risk of freezing has been prevented.</p>
<p><i>Early Start Duration</i> - if greater than 0, begins heating earlier than scheduled so that the scheduled temperature is reached by the set time.</p>
<p><i>Heating Manual Override</i> - switch the heating on and control to the desired temperature, switched-off when the next timed period begins.</p>
<p><i>Heating Manual Override Temperature</i> - used to set the desired manual override temperature.</p>
</div>
  )rawliteral";
  webpage += CreateHtmlFooter(version, year);
  return webpage;
}
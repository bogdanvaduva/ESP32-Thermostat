//#########################################################################################
// inspired by David Bird work
String CreateHtmlHeader(String sitetitle, String page, bool refreshMode) {
  String webpage = "";            // General purpose variable to hold HTML code for display
  webpage = "<!DOCTYPE html>";
  webpage += "<html>";
  webpage += "<head>";
  webpage += "  <title>" + sitetitle + "</title>";
  webpage += "<meta charset='UTF-8'>";
  if (refreshMode) webpage += "<meta http-equiv='refresh' content='5'>";  // 5-secs refresh time, test needed to prevent auto updates repeating some commands
  webpage += "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  webpage += "  <script src=\"https://code.jquery.com/jquery-3.2.1.min.js\"></script>";
  webpage += "  <link rel=\"icon\" href=\"data:,\">";
  webpage += "  <style>";
  webpage += CreateHtmlCss();
  webpage += "  </style>";
  webpage += "</head>";
  webpage += "<body>";
  webpage += "  <div class=\"topnav\">";
  webpage += "<a href='home' ";
  if (page == "home") webpage += "class='active'";
  webpage += ">Home</a>";
  webpage += "<a href='graphs' ";
  if (page == "graphs") webpage += "class='active'";
  webpage += ">Graphs</a>";
  webpage += "<a href='schedule' ";
  if (page == "schedule") webpage += "class='active'";
  webpage += ">Schedules</a>";
  webpage += "<a href='setup' ";
  if (page == "setup") webpage += "class='active'";
  webpage += ">Setup</a>";
  webpage += "<a href='help' ";
  if (page == "help") webpage += "class='active'";
  webpage += ">Help</a>";
  webpage += "  </div>";
  return webpage;
}
//#########################################################################################
// inspired by David Bird work
String CreateHtmlFooter(String version, String Year) {
  String webpage = "";            // General purpose variable to hold HTML code for display
  webpage += "<footer>";
  webpage += "<p class='ps'><i>Copyright &copy;&nbsp;D. L. Bird, B. Vaduva " + String(Year) + ". Version - " + version + ".</i></p>";
  webpage += "</footer>";
  webpage += "</body></html>";
  return webpage;
}

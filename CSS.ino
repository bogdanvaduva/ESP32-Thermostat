String CreateHtmlCss(){
  String css = R"rawliteral(html {
      font-family: Arial, Helvetica, sans-serif; 
      display: inline-block; 
      text-align: center;
    }

    h1 {
      font-size: 1.8rem; 
      color: white;
    }

    p { 
      font-size: 1.4rem;
    }

    .topnav { 
      overflow: hidden; 
      background-color: #0A1128;
    }

    .topnav a {
      float:left;
      color:white;
      text-align:center;
      padding:1em 1.14em;
      text-decoration:none;
      font-size:1.3em;
    }
    
    .topnav a:hover {
      background-color: #c2cae5;
      color:black;
    }

    .topnav a.active {
      background-color: #3c668b;
      color:white;
    }

    body {  
      margin: 0;
    }

    .content { 
      padding: 5%;
    }

    .card-grid { 
      max-width: 800px; 
      margin: 0 auto; 
      display: grid; 
      grid-gap: 2rem; 
      grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
    }

    .card { 
      background-color: white; 
      box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);
    }

    .card-title { 
      font-size: 1.2rem;
      font-weight: bold;
      color: #034078;
    }

    input[type=submit] {
      border: none;
      color: #FEFCFB;
      background-color: #034078;
      padding: 15px 15px;
      text-align: center;
      text-decoration: none;
      display: inline-block;
      font-size: 16px;
      width: 100px;
      margin-right: 10px;
      border-radius: 4px;
      transition-duration: 0.4s;
    }

    input[type=submit]:hover {
      background-color: #1282A2;
    }

    input[type=text], input[type=number], select {
      width: 50%;
      padding: 12px 20px;
      display: inline-block;
      border: 1px solid #ccc;
      border-radius: 4px;
      box-sizing: border-box;
    }

    .div-table {
      display: table;         
      width: 100% !important;         
      background-color: transparent;         
      border: 0px solid #ffffff;         
      border-spacing: 5px; /* cellspacing:poor IE support for  this */
    }

    .div-table-row {
      display: table-row;
      width: auto;
      clear: both;
    }

    .div-table-col {
      float: left; /* fix for  buggy browsers */
      display: table-column;         
      background-color: transparent;
      vertical-align: middle;
      text-align: center;
    }

    label {
      font-size: 1.2rem; 
    }

    .value{
      font-size: 1.2rem;
      color: #1282A2;  
    }

    .state {
      font-size: 1.2rem;
      color: #1282A2;
    }

    button {
      border: none;
      color: #FEFCFB;
      padding: 15px 32px;
      text-align: center;
      font-size: 16px;
      width: 100px;
      border-radius: 4px;
      transition-duration: 0.4s;
    }

    .button-on {
      background-color: #034078;
    }

    .button-on:hover {
      background-color: #1282A2;
    }

    .button-off {
      background-color: #858585;
    }

    .button-off:hover {
      background-color: #252524;
    }

    .wifi {
      padding:3px;
      position:relative;
      top:1em;
      left:0.36em;
    }

    .wifi, .wifi:before {
      display:inline-block;
      border:9px double transparent;
      border-top-color:currentColor;
      border-radius:50%;
    }

    .wifi:before {
      content:'';
      width:0;
      height:0;
    }

    .numberCircle {
      border-radius:50%;
      width:2.7em;
      height:2.7em;
      border:0.11em solid blue;
      padding:0.2em;
      color:blue;
      text-align:center;
      font-size:3em;
      display:inline-flex;
      justify-content:center;
      align-items:center;
    }

    .centre, #outer {
      margin-left:auto;
      margin-right:auto;
    }
    )rawliteral";
  return css;
}
var xhrRequest = function (url, type, data, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.setRequestHeader('Content-Type', 'application/json; charset=UTF-8');
  xhr.send(data);
};

function getStatus() {
  var dictionary = {};
  var url = "http://192.168.0.1/virb";
  var status = "{\"command\":\"status\"}";
  var features = "{\"command\":\"features\"}";

  var batteryLevel = 0;
  var altitude = "";
  var speed = "";
  var gps = "";
  var state = "";
  var units = "";

  xhrRequest(url, "POST", status,
  function(responseText) {
    var json = JSON.parse(responseText);
    if (!json.hasOwnProperty('altitude')) {
      altitude = 0;
    } else {
      altitude = parseInt(json.altitude);
    }

    if (!json.hasOwnProperty('speed')) {
      speed = 0;
    } else {
      speed = parseInt(json.speed);
    }

    batteryLevel = json.batteryLevel;
    state = json.state;

    xhrRequest(url, "POST", features,
    function(responseText) {
      var json = JSON.parse(responseText);

      for (var i = 0; i < json.features.length; i++) {
        if (json.features[i].feature === "units") {
          units = json.features[i].value;
        }

        if (json.features[i].feature === "gps") {
          gps = json.features[i].value;
        }
      }

      if (units === "Metric") {
        altitude += " m";
        speed += " km/h";
      } else {
        altitude += " ft";
        speed += " mph";
      }

      dictionary = {
        "BATTERY": batteryLevel,
        "ALTITUDE": altitude,
        "SPEED": speed,
        "GPS": gps,
        "STATE": state
      };

      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log("Data sent!");
        },
        function(e) {
          console.log("Error sending data!");
        }
      );

    });

  });




}

function request(action) {
  var url = "http://192.168.0.1/virb";
  var status = "{\"command\":\"status\"}";
  var features = "{\"command\":\"features\"}";
  var command;

  switch(action) {
    case "PHOTO":
      command = "{\"command\":\"snapPicture\"}";
      xhrRequest(url, "POST", command,
      function(responseText) {
        console.log(command + " executed!");
        getStatus();
      });
      break;
    case "GPS":
      xhrRequest(url, "POST", features,
      function(responseText) {
        var json = JSON.parse(responseText);
        for (var i = 0; i < json.features.length; i++) {
          if (json.features[i].feature === "gps") {
            if (json.features[i].value === "on") {
              command = "{\"command\":\"updateFeature\",\"feature\": \"gps\",\"value\": \"off\" }";
            } else {
              command = "{\"command\":\"updateFeature\",\"feature\": \"gps\",\"value\": \"on\" }";
            }
          }
        }
        xhrRequest(url, "POST", command,
          function(responseText) {
          console.log(command + " executed!");
          getStatus();
        });
      });
      break;
    case "RECORD":
      xhrRequest(url, "POST", status,
      function(responseText) {
        var json = JSON.parse(responseText);
        if (json.state === "idle") {
          command = "{\"command\":\"startRecording\"}";
        }
        else {
          command = "{\"command\":\"stopRecording\"}";
        }

        xhrRequest(url, "POST", command,
        function(responseText) {
          console.log(command + " executed!");
          getStatus();
        });
      });
      break;
    case "STATUS":
      getStatus();
      break;
  }
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready',
function(e) {
  getStatus();
}
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
function(e) {
  for (var property in e.payload) {
    console.log(property);
    request(property);
  }
}
);

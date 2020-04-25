function myFunction() {
  
  var files = DriveApp.getFolderById('xxxxxxxxxxx').getFiles(); //replace xx with your folder id
  var file = files.next();
 
  var timestamp = file.getDateCreated();

  var real_timestamp = Utilities.formatDate(timestamp, "Asia/Bangkok", "yyyy/MM/dd hh:mm:ss");
  var last_url = file.getDownloadUrl();
  var sheet = SpreadsheetApp.openById('xxxxxxxxxxxxx').getActiveSheet(); //replace xx with your google sheet id
  sheet.appendRow([real_timestamp, last_url, '0', '1']);
  
  //remove duplicate row
  var data = sheet.getDataRange().getValues();
  var newData = [];
  for (var i in data) {
    var row = data[i];
    var duplicate = false;
    for (var j in newData) {
      if (row.join() == newData[j].join()) {
        duplicate = true;
      }
    }
    if (!duplicate) {
      newData.push(row);
    }
  }
  sheet.clearContents();
  sheet.getRange(1, 1, newData.length, newData[0].length).setValues(newData);
  
}

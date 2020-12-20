function rowStyle(row, index) {
  var status = [
    "waiting",
    "scanned",
    "unknown",
    "invalid"
  ];

  return {
    classes: status[row.status]
  };
}

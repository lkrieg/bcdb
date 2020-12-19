var tbody;

function update(data)
{
	clear();
	data.forEach(function(entry) {
		insert(entry);
	});
}

function poll(url, callback)
{
	var xhr = new XMLHttpRequest();

	xhr.open("GET", url, true);
	xhr.onload = function (e) {
		if (xhr.readyState === 4 && xhr.status === 200)
			callback(JSON.parse(xhr.responseText));
	};

	xhr.send(null);
}

function insert(entry)
{
	var status;
	var infotext;
	var tbody, row;

	status    = entry.status;
	infotext  = (status == 0) ? "waiting" :
	            (status == 1) ? "scanned" :
	            (status == 2) ? "invalid" :
	                            "unknown" ;

	tbody = document.getElementById("view-data");
	row = document.createElement("tr");
	row.classList.add(infotext);

	row.appendChild(column(entry.recipient));
	row.appendChild(column(entry.barcode));
	row.appendChild(column(infotext));

	tbody.appendChild(row);
}

function column(val)
{
	var col, text;

	col  = document.createElement("td");
	text = document.createTextNode(val);

	col.appendChild(text);
	return col;
}

function clear()
{
	var tbody;

	tbody = document.getElementById("view-data");
	tbody.innerHTML = "";
}

function main()
{
	// TODO: Get timestamp from server
	poll("/list", function(data) {
		update(data);
	});
}

window.onload = main;

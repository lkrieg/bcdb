var time; // last database update

function poll(url, callback)
{
	var xhr = new XMLHttpRequest();

	xhr.open("GET", url, true);
	xhr.onload = function (e) {
		if ((xhr.readyState === 4)
		&& ((xhr.status === 200 && xhr.response.length > 0)))
			callback(xhr.responseText);
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

function getlist()
{
	poll("/list", function(data) {
		list = JSON.parse(data);
		clear(); // TODO: Efficiency
		list.forEach(function(entry) {
			insert(entry);
		});
	});
}

function update()
{
	var list;

	poll("/time", function(updated) {
		setTimeout(update, 500);
		if (updated > time) {
			time = updated;
			getlist();
		}
	});
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
	time = 0;
	getlist();
	setTimeout(update, 500);
}

window.onload = main;

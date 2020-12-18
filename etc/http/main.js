var tbody;

function update(list)
{
	clear();
	list.forEach(function(entry) {
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
	var tr, td;
	var text;

	tr = document.createElement("tr");
	tr.classList.add((entry.status == 0) ? "status-ok"   :
	                 (entry.status == 1) ? "status-warn" :
	                                       "status-error");
	for (var key in entry) {
		if (!entry.hasOwnProperty(key))
			continue;

		td    = document.createElement("td");
		text  = document.createTextNode(entry[key]);

		td.classList.add(key);
		td.appendChild(text);
		tr.appendChild(td);
	}

	tbody.appendChild(tr);
}

function clear()
{
	tbody.innerHTML = "";
}

function main()
{
	tbody = document.getElementById("view-data");

	poll("/list", function(list) {
		update(list);
	});
}

window.onload = main;

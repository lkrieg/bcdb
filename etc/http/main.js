var time; // last database update

function rowStyle(row, index)
{
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

function update()
{
	var newtime;

	poll("/time", function(data) {
		setTimeout(update, 1000);
		newtime = parseInt(data);
		if (newtime > time) {
			time = newtime;
			refreshTable();
		}
	});
}

function refreshTable()
{
	$('#table').bootstrapTable('refresh', {silent: true});
}

function main()
{
	time = 1;
	setTimeout(update, 500);
}

window.onload = main;

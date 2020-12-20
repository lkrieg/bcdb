var time; // last database update

function poll(url, callback)
{
	var xhr, length;
	var status, data;

	var xhr = new XMLHttpRequest();
	xhr.open("GET", url, true);
	xhr.onload = function (e) {
		if (xhr.readyState === 4) {
			status = xhr.status;
			data   = xhr.responseText;
			length = xhr.response.length;
			callback(status, data, length);
		}
	};

	xhr.send(null);
}

function update()
{
	var newtime;

	poll("/time", function(status, data, length) {
		setTimeout(update, 500);
		if (status === 200 && length > 0) {
			newtime = parseInt(data);
			if (newtime > time) {
				time = newtime;
				refreshTable();
			}
		}
	});
}

function main()
{
	time = 1;
	setTimeout(update, 500);

}

function setGroup(value, idx, data)
{
	return "";
}

function rowStyle(row, index)
{
	var status = [
		"table-default",
		"table-success",
		"table-warning",
		"table-danger"
	];

	return {
		classes: status[row.status]
	};
}

function refreshTable()
{
	$('#table').bootstrapTable('refresh', {silent: true});
}

// Quick fix to prevent flash of unstyled content
// Also replaces status codes with human readable strings
$('#table').on('post-body.bs.table', function (data) {
	var status;

	$('tbody .status').each(function() {
		status = parseInt($(this).text());
		$(this).text((status == 0) ? "Waiting" :
		             (status == 1) ? "Scanned" :
		             (status == 2) ? "Invalid" :
		                             "Unknown");
	});

	$('body').show();
});

window.onload = main;

$(document).ready(function(){
	$('#btn_login').click(function() {
		login();
	});
	$(".input-block-level").keypress(function(event) {
		if ( event.which == 13 ) {
			login();
		}
	});
	var username = getUrlParam("username");
	if (username) {
		$("#username").val(username);
	}
});

function login(){
	var username=$('#username').val();
	var password=$('#password').val();
	var alert = $("#alert");
	if ( username == "" || password == "" ) {
		bsalert(alert, "error", "Form incomplete.")
		return;
	}
	$.getJSON('/cmapi/developer/login', {username:username, password:password}, function(json){
		var err = json.error;
		if (err==0){
			bsalert(alert, "success", "Sign in succeed and redirect to main page...");
			window.location.href="main.html";
		}else{
			bsalert(alert, "error", "Sign in failed. Error=" + err);
		}
	}).error(function() {
	  	bsalert(alert, "error", "Ajax error.");
	});
}
$(document).ready(function(){
	$('#btn_register').click(function() {
		register();
	});
	$(".input-block-level").keypress(function(event) {
		if ( event.which == 13 ) {
			register();
		}
	});
});

function register(){
	var username=$('#username').val();
	var password=$('#password').val();
	var alert = $("#alert");
	if ( username == "" || password == "" ) {
		bsalert(alert, "error", "Form incomplete.")
		return;
	}
	$.getJSON('/cmapi/developer/register', {username:username, password:password}, function(json){
		var err = json.error;
		if (err==0){
			bsalert(alert, "success", "Sign up succeed");
			setTimeout("window.location.href='login.html'",1000);
		}else{
			bsalert(alert, "error", "Sign up failed. Error=" + err);
		}
	}).error(function() {
	  	bsalert(alert, "error", "Ajax error.");
	});
}
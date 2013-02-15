var appname;
$(document).ready(function(){
	appname = getUrlParam("appname");
    $("#title").text("App:"+appname);
    
   	$("#edit").click(function() {
        $("#input_edit").val(appname);
        $("#modal_edit").modal( {
            keyboard: false,
            backdrop: "static"}
        );
    });

    $("#secret").click(function() {
        $("#modal_secret").modal( {
            keyboard: false,
            backdrop: "static"}
        );
    });
});

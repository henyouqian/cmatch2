
$(document).ready(function(){
    var username = getCookie("username");
    if (username) {
        $("#username_span").text(username);
    }

    $.getJSON("/cmapi/developer/listapp", function(json) {
        var err = json.error;
        if (err==0) {
            var apps = json.apps;
            for (appid in apps) {
                var appname = apps[appid];
                var href = "app.html?appid="+appid+"&appname="+appname;
                $("#ul_apps").append("<li><a href='"+href+"'>"+appname+"</a></li>");
            }
        }else{
            window.location.href = "login.html";
            //alert(err);
        }
    })

    $("#a_addapp").click(function() {
        $("#input_appname").val("");
        $("#addapp_modal").modal( {
            keyboard: false,
            backdrop: "static"}
        );
    });

    $("#btn_add_app").click(function(){
        var appname = $("#input_appname").val();
        var alt = $("#addapp_alert");
        if (appname == "") {
            bsalert(alt, "error", "Need app name");
            return;
        }
        $.getJSON("/cmapi/developer/addapp", {"appname":appname}, function(json) {
            if (json.appid) {
                var href = "app.html?appid="+appid+"&appname="+appname;
                $("#ul_apps").append("<li><a href='"+href+"'>"+appname+"</a></li>");
                $("#addapp_modal").modal("hide");
            }
            if (json.error) {
                bsalert(alt, "error", "error:"+json.error);
            }
        });
    });
});
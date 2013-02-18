var appname;
var appid;
$(document).ready(function(){
	appname = getUrlParam("appname");
    appid = getUrlParam("appid");
    if (!appname || !appid) {
        window.location.href = "main.html";
        return;
    }
    $("#title").text("App:"+appname);
    
    //show edit app name dialog
   	$("#edit").click(function() {
        $("#input_edit").val(appname);
        $("#modal_edit").modal( {
            keyboard: false,
            backdrop: "static"}
        );
    });

    //show secret
    $("#secret").click(function() {
        $.getJSON('/cmapi/developer/appsecret', {appid:appid}, function(json){
            var err = json.error;
            if (err==0){
                $("#modal_secret").modal( {
                    keyboard: false,
                    backdrop: "static"}
                );
                $("#secret_text").text(json.secret);
            }else{
                alert("Get secret error:"+err);
                errorProc(err);
            }
        });
    });

    //submit edit app name
    $("#btn_edit").click(function() {
        var appname = $("#input_edit").val();
        if (!appname) {
            bsalert($("#edit_alert"), "error", "Fill app name.")
            return;
        }
        $.getJSON('/cmapi/developer/editapp', {appid:appid, appname:appname}, function(json){
            var err = json.error;
            if (err==0){
                $("#modal_edit").modal("hide");
                window.appname = appname;
                $("#title").text("App:"+appname);
            }else{
                alert("Edit app error:"+err);
                errorProc(err);
            }
        });
    });

    //list games
    $.getJSON("/cmapi/developer/listgames", {appid:appid}, function(json) {
        var err = json.error;
        if (err==0) {
            var games = json.games;
            for (gameid in games) {
                var gamename = games[gameid];
                appendGame(gameid, gamename);
            }
        }else{
            alert("List game error:"+err);
            errorProc(err);
        }
    })

    //show add game dialog
    $("#add_game_dlg").click(function() {
        $("#input_addgame").val("");
        $("#addgame_modal").modal( {
            keyboard: false,
            backdrop: "static"}
        );
    });

    //submit add game
    $("#btn_addgame").click(function() {
        var gamename = $("#input_addgame").val();
        if (!gamename) {
            alert("error: Need game name");
            return;
        }
        $.getJSON('/cmapi/developer/addgame', {appid:appid, gamename:gamename}, function(json){
            var err = json.error;
            if (err==0){
                $("#addgame_modal").modal("hide");
                appendGame(json.gameid, gamename);
            }else{
                alert("Add game error:"+err);
                errorProc(err);
            }
        });
    });

    //submit edit game
    $("#btn_editgame").click(function() {
        var gamename = $("#input_editgame").val();
        if (!gamename) {
            alert("error: Need game name");
            return;
        }
        $.getJSON('/cmapi/developer/editgame', {gameid:gameid, gamename:gamename}, function(json){
            var err = json.error;
            if (err==0){
                $("#editgame_modal").modal("hide");
                updateGame(gameid, gamename);
            }else{
                alert("Edit game error:"+err);
                errorProc(err);
            }
        });
    });

    //show add match dialog
    $("#add_match_dlg").click(function() {
        $("#input_addmatch").val("");
        $("#addmatch_modal").modal( {
            keyboard: false,
            backdrop: "static"}
        );
    });
});

function errorProc(err) {
    if (err == "CMERR_AUTH") {
        window.location.href = "login.html";
    }
}

function appendGame(gameid, gamename) {
    var aid = "_a"+gameid;
    $("#tbl_game").append("<tr><td>"+gameid+"</td> <td><a id="+aid+" href='javascript:void(0)' gameid="+gameid+">"+gamename+"</a></td></tr>");
    $("#"+aid).click(function(){
        window.gameid = gameid;
        $("#input_editgame").val($(this).text());
        $("#editgame_modal").modal( {
            keyboard: false,
            backdrop: "static"}
        );
    });
}

function updateGame(gameid, gamename) {
    $("#_a"+gameid).text(gamename);
}
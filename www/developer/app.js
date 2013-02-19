var appname;
var appid;
var matchesPerPage = 10;
var currPage = 0;
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
    $.getJSON("/cmapi/developer/listgame", {appid:appid}, function(json) {
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

    //list matches
    listMatch();

    //show add match dialog
    $("#add_match_dlg").click(function() {
        $("#input_addmatch").val("");
        $("#addmatch_modal").modal( {
            keyboard: false,
            backdrop: "static"}
        );
        $(".form_datetime").val("");
    });

    //match begin datapicker
    $(".form_datetime").datetimepicker({
        format: 'yyyy-mm-dd hh:ii',
        autoclose: true,
        todayHighlight: true,
        todayBtn: true,
    });

    //submit add match
    $("#btn_addmatch").click(function() {
        var matchname = $("#input_addmatch").val();
        var gameid = $("#game_select").val();
        var begin = $("#dtpicker_begin").val();
        var end = $("#dtpicker_end").val();
        if (!begin || !end) {
            alert("Need begin end");
            return;
        }
        if (begin >= end) {
            alert("begin >= end");
            return;
        }
        $.getJSON('/cmapi/developer/addmatch', 
                {appid:appid, gameid:gameid, begin:begin, end:end, matchname:matchname}, function(json){
            var err = json.error;
            if (err==0){
                $("#addmatch_modal").modal("hide");
                var select = $("#game_select")[0];
                var gamename = select.options[select.selectedIndex].text;
                prependMatch(json.matchid, matchname, gamename, begin, end);
            }else{
                alert("Add match error:"+err);
                errorProc(err);
            }
        });
    });

    //match page
    $("#match_prev_page").click(function(){
        if (currPage==0)
            return;
        currPage -= 1;
        if (currPage==0)
            $("#match_prev_page").addClass("disabled");
        $("#page_num").text(currPage+1);
        listMatch();
    });
    $("#match_next_page").click(function(){
        if ($("#match_next_page").hasClass("disabled"))
            return;
        if (currPage==0)
            $("#match_prev_page").removeClass("disabled");
        currPage += 1;
        $("#page_num").text(currPage+1);
        listMatch();
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
    var optionid = "_gameoption"+gameid;
    $("#game_select").append("<option id='"+optionid+"' value ='"+gameid+"'>"+gamename+"</option>");
}

function updateGame(gameid, gamename) {
    $("#_a"+gameid).text(gamename);
    $("#_gameoption"+gameid).val(gameid).text(gamename);
}

function appendMatch(matchid, matchname, gamename, begintime, endtime) {
    $("#tbl_match").append(" <tr class='match_row'>"+
                                "<td><a href='javascript:void(0)'>Edit</a></td>"+
                                "<td>"+matchid+"</td>"+
                                "<td>"+matchname+"</td>"+
                                "<td>"+gamename+"</td>"+
                                "<td>"+begintime+"</td>"+
                                "<td>"+endtime+"</td>"+
                            "</tr>");
}

function prependMatch(matchid, matchname, gamename, begintime, endtime) {
    $("#tbl_match_header").after(" <tr class='match_row'>"+
                                "<td><a href='javascript:void(0)'>Edit</a></td>"+
                                "<td>"+matchid+"</td>"+
                                "<td>"+matchname+"</td>"+
                                "<td>"+gamename+"</td>"+
                                "<td>"+begintime+"</td>"+
                                "<td>"+endtime+"</td>"+
                            "</tr>");
}

function listMatch() {
    var oldRows = $(".match_row");
    $.getJSON("/cmapi/developer/listmatch", {appid:appid, offset:currPage*matchesPerPage, limit:matchesPerPage+1}, function(json) {
        var err = json.error;
        $("#match_next_page").addClass("disabled");
        if (err==0) {
            var matches = json.matches;
            for (i in matches) {
                if (i == matchesPerPage) {
                    $("#match_next_page").removeClass("disabled");
                    break;
                }
                var match = matches[i];
                appendMatch(match[0], match[1], match[2], match[3], match[4]);
            }
        }else{
            alert("List game error:"+err);
            errorProc(err);
        }
        oldRows.remove();
    })
}
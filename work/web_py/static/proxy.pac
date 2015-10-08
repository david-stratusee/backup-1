// Proxy Auto-Config file generated by autoproxy2pac, 2014-12-31 21:18:00
function FindProxyForURL(url, host) {
    var autoproxy = 'SOCKS 127.0.0.1:8099';
    var defaultproxy = 'DIRECT';
    if (isPlainHostName(host) ||
        host.search(/127.|10.|172.16.|192.168.|localhost./i) == 0) {
        return defaultproxy;
    } else {
        var autoproxy_ret = FindProxyForURLByAutoProxy(url, host)
        if (autoproxy_ret != 0) {
            return autoproxy;
        } else {
            return defaultproxy;
        }
    }
}

var autoproxy_host_new = {
    "all-that-is-interesting.com": 1,
    "amazon.com": 1,
    "android.com": 1,
    "anonymizer.com": 1,
    "aolchannels.aol.com": 1,
    "appledaily.com": 1,
    "asianews.it": 1,
    "bbc.co.uk": 1,
    "bbc.in": 1,
    "bestvpnservice.com": 1,
    "cdnews.com.tw": 1,
    "centurys.net": 1,
    "china-week.com": 1,
    "chinesedailynews.com": 1,
    "chinesenewsnet.com": 1,
    "chrome.com": 1,
    "chromeadblock.com": 1,
    "cn.dayabook.com": 1,
    "cms.gov": 1,
    "cmule.com": 1,
    "cn.giganews.com": 1,
    "cna.com.tw": 1,
    "cn.voa.mobi": 1,
    "cnn.com": 1,
    "codeboxapp.com": 1,
    "codeshare.io": 1,
    "compileheart.com": 1,
    "dailynews.sina.com": 1,
    "dl.box.net": 1,
    "developers.box.net": 1,
    "daylife.com": 1,
    "date.fm": 1,
    "doxygen.org": 1,
    "dropbox.com": 1,
    "dropboxusercontent.com": 1,
    "dyndns.org": 1,
    "e-info.org.tw": 1,
    "earthquake.usgs.gov": 1,
    "ebookbrowse.com": 1,
    "ebookee.com": 1,
    "emacsblog.org": 1,
    "eltondisney.com": 1,
    "emule-ed2k.com": 1,
    "etaiwannews.com": 1,
    "extremetube.com": 1,
    "filefactory.com": 1,
    "flickr.com": 1,
    "fflick.com": 1,
    "forum.baby-kingdom.com": 1,
    "free-gate.org": 1,
    "free-ssh.com": 1,
    "gist.github.com": 1,
    "gmail.com": 1,
    "gstatic.com": 1,
    "hk.knowledge.yahoo.com": 1,
    "hk.myblog.yahoo.com": 1,
    "hk.news.yahoo.com": 1,
    "hk.search.yahoo.com": 1,
    "hk.video.news.yahoo.com": 1,
    "hk.yahoo.com": 1,
    "hkdailynews.com.hk": 1,
    "home.sina.com": 1,
    "linux-engineer.net": 1,
    "linuxconfig.org": 1,
    "linuxreviews.org": 1,
    "linuxtoy.org": 1,
    "magazines.sina.com.tw": 1,
    "my.opera.com": 1,
    "myeclipseide.com": 1,
    "myspace.com": 1,
    "netflix.com": 1,
    "newcenturynews.com": 1,
    "news.msn.com.tw": 1,
    "news.sina.com.hk": 1,
    "news.sina.com.tw": 1,
    "nurgo-software.com": 1,
    "onedrive.live.com": 1,
    "openvpn.net": 1,
    "pchome.com.tw": 1,
    "perlhowto.com": 1,
    "photo.utom.us": 1,
    "photofocus.com": 1,
    "photos.dailyme.com": 1,
    "picturesocial.com": 1,
    "power.com": 1,
    "powerapple.com": 1,
    "proxy.org": 1,
    "proxypy.net": 1,
    "proxifier.com": 1,
    "purepdf.com": 1,
    "purevpn.com": 1,
    "python.com": 1,
    "python.com.tw": 1,
    "qq.co.za": 1,
    "read100.com": 1,
    "readingtimes.com.tw": 1,
    "readmoo.com": 1,
    "redtube.com": 1,
    "referer.us": 1,
    "rhcloud.com": 1,
    "shadowsocks.org": 1,
    "simplecd.org": 1,
    "simpleproductivityblog.com": 1,
    "skyhighpremium.com": 1,
    "slickvpn.com": 1,
    "slideshare.net": 1,
    "snapchat.com": 1,
    "sourceforge.net": 1,
    "southnews.com.tw": 1,
    "squarespace.com": 1,
    "static.apple.nextmedia.com": 1,
    "staticflickr.com": 1,
    "taiwandaily.net": 1,
    "tidyread.com": 1,
    "time.com": 1,
    "times.hinet.net": 1,
    "tinychat.com": 1,
    "tmagazine.com": 1,
    "torvpn.com": 1,
    "turbobit.net": 1,
    "turbotwitter.com": 1,
    "tw.news.yahoo.com": 1,
    "tw.voa.mobi": 1,
    "tw.yahoo.com": 1,
    "twitonmsn.com": 1,
    "twitpic.com": 1,
    "twitreferral.com": 1,
    "twitstat.com": 1,
    "twittbot.net": 1,
    "twitter.com": 1,
    "twitter4j.org": 1,
    "twittercounter.com": 1,
    "twitterfeed.com": 1,
    "twittergadget.com": 1,
    "twitterkr.com": 1,
    "twittermail.com": 1,
    "twittertim.es": 1,
    "twitthat.com": 1,
    "twitturk.com": 1,
    "twitturly.com": 1,
    "twitvid.com": 1,
    "twitzap.com": 1,
    "ultravpn.fr": 1,
    "unicode.org": 1,
    "upcoming.yahoo.com": 1,
    "urlparser.com": 1,
    "uushare.com": 1,
    "video.aol.ca": 1,
    "video.aol.co.uk": 1,
    "video.aol.com": 1,
    "video.yahoo.com": 1,
    "voachinese.com": 1,
    "voachineseblog.com": 1,
    "voanews.com": 1,
    "web2project.net": 1,
    "wiki.cnitter.com": 1,
    "wiki.jqueryui.com": 1,
    "wiki.oauth.net": 1,
    "wordpress.com": 1,
    "wujie.net": 1,
    "www.aolnews.com": 1,
    "www.skype.com": 1,
    "xbookcn.com": 1,
    "xinhuanet.org": 1,
    "youtu.be": 1,
    "freeyoutubeproxy.net": 1,
    "listentoyoutube.com": 1,
    "youtube-nocookie.com": 1,
    "youtube.com": 1,
    "youtubecn.com": 1,
    "youversion.com": 1,
    "zdnet.com.tw": 1,
    "wikipedia.org": 1,
    "zh.wikibooks.org": 1,
    "zh.wikinews.org": 1,
    "zh.wikisource.org": 1,
    "cdnjs.cloudflare.com": 1,
    "zlib.net": 1,
    "ziplib.com": 1,
    "tvboxnow.com": 1,
    "stratusee.com": 1,
    "curl.haxx.se": 1,
    "tldp.org": 1,
};

function FindProxyForURLByAutoProxy(url, host) {
    if (host.search(/google|facebook/i) >= 0) {
        return 1;
    }

    var host_array = host.split('.');
    var i = 0;
    var len = host_array.length;
    var tmp = host_array[len - 1];
    for (i = len - 2; i >= 0; i--) {
        tmp = host_array[i] + '.' + tmp;
        if (autoproxy_host_new.hasOwnProperty(tmp)) {
            return 1;
        }
    }

    return 0;
}
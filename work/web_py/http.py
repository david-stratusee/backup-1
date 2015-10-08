#!/usr/bin/env python

import web, os

urls = (
    '/', 'index',
    '/images/(.*)', 'images'
)


class index:
    def GET(self):
        fp = open("index.html")
        lines=fp.readlines()
        fp.close()
        return lines


class images:
    def GET(self,name):
        ext = name.split(".")[-1] # Gather extension

        cType = {
            "png":"images/png",
            "jpg":"images/jpeg",
            "gif":"images/gif",
            "ico":"images/x-icon"            }

        if name in os.listdir('images'):  # Security
            web.header("Content-Type", cType[ext]) # Set the Header
            return open('images/%s'%name,"rb").read() # Notice 'rb' for reading images
        else:
            return name + "no find"
            #raise web.notfound()


if __name__ == "__main__":
    app = web.application(urls, globals())
    app.run()

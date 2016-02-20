package main

import (
	"github.com/PuerkitoBio/goquery"
	"github.com/jordan-wright/email"

	"bytes"
	"io/ioutil"
	"log"
	"net/http"
	"net/smtp"
	"strings"
)

func main() {
	resp, err := http.Get("http://www.cnblogs.com/jasondan/p/3497757.html")
	if err != nil {
		log.Fatal(err)
	}
	defer resp.Body.Close()

	content, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		log.Fatal(err)
	}

	doc, err := goquery.NewDocumentFromReader(bytes.NewReader(content))
	if err != nil {
		log.Fatal(err)
	}

	header := ""
	doc.Find(`.postTitle`).Each(func(i int, s *goquery.Selection) {
		header = strings.TrimSpace(s.Text())
	})

	e := email.NewEmail()
	e.From = "cnblogs <zlyang_bj@163.com>"
	e.To = []string{sendTo}
	e.Subject = header
	e.HTML = []byte(content)
	e.Send("smtp.163.com:25", smtp.PlainAuth("", sendFrom, sendFromPassword, "smtp.163.com"))
}

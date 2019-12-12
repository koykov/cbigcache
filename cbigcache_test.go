package cbigcache

import (
	"bytes"
	"math/rand"
	"strings"
	"sync"
	"testing"
	"time"
)

func randKey(n int) string {
	rand.Seed(time.Now().UnixNano())
	chars := []rune("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789")
	var b strings.Builder
	for i := 0; i < n; i++ {
		b.WriteRune(chars[rand.Intn(len(chars))])
	}
	return b.String()
}

func TestIOConsecutive(t *testing.T) {
	config := DefaultConfig(1 * time.Minute)
	config.Shards = 8
	config.MaxSize = 10 * Megabyte

	cbc, _ := NewCBigCache(config)

	data := []byte(`{
 "glossary": {
     "title": "example glossary",
		"GlossDiv": {
         "title": "S",
			"GlossList": {
             "GlossEntry": {
                 "ID": "SGML",
					"SortAs": "SGML",
					"GlossTerm": "Standard Generalized Markup Language",
					"Acronym": "SGML",
					"Abbrev": "ISO 8879:1986",
					"GlossDef": {
                     "para": "A meta-markup language, used to create markup languages such as DocBook.",
						"GlossSeeAlso": ["GML", "XML"]
                 },
					"GlossSee": "markup"
             }
         }
     }
 }
}`)
	err0 := cbc.Set("a7S77X6EoeSwZ2FNhFG8", data)
	if err0 != nil {
		t.Error(err0)
	}

	dataRecv, n, err1 := cbc.Get("a7S77X6EoeSwZ2FNhFG8")
	if err1 != nil {
		t.Error(err1)
	} else {
		if !bytes.Equal(data, dataRecv) {
			t.Error("received data isn't equal with original")
		}
		if n != uint(len(data)) {
			t.Error("received", n, "bytes, but expected", len(data))
		}
	}

	_ = cbc.Free()
}

func TestIOParallel(t *testing.T) {
	config := DefaultConfig(10 * time.Second)
	config.Shards = 16
	config.ForceSet = true
	config.MaxSize = 8 * Megabyte
	config.VerboseLevel = VerboseLevelDebug3
	cbc, _ := NewCBigCache(config)

	dataPool := [][]byte{
		[]byte(`{"firstName":"John","lastName":"Smith","isAlive":true,"age":27,"address":{"streetAddress":"21 2nd Street","city":"New York","state":"NY","postalCode":"10021-3100"},"phoneNumbers":[{"type":"home","number":"212 555-1234"},{"type":"office","number":"646 555-4567"},{"type":"mobile","number":"123 456-7890"}],"children":[],"spouse":null}`),
		[]byte(`{"$schema":"http://json-schema.org/schema#","title":"Product","type":"object","required":["id","name","price"],"properties":{"id":{"type":"number","description":"Product identifier"},"name":{"type":"string","description":"Name of the product"},"price":{"type":"number","minimum":0},"tags":{"type":"array","items":{"type":"string"}},"stock":{"type":"object","properties":{"warehouse":{"type":"number"},"retail":{"type":"number"}}}}}`),
		[]byte(`{"id":1,"name":"Foo","price":123,"tags":["Bar","Eek"],"stock":{"warehouse":300,"retail":20}}`),
		[]byte(`{"first name":"John","last name":"Smith","age":25,"address":{"street address":"21 2nd Street","city":"New York","state":"NY","postal code":"10021"},"phone numbers":[{"type":"home","number":"212 555-1234"},{"type":"fax","number":"646 555-4567"}],"sex":{"type":"male"}}`),
		[]byte(`{"fruit":"Apple","size":"Large","color":"Red"}`),
		[]byte(`{"quiz":{"sport":{"q1":{"question":"Which one is correct team name in NBA?","options":["New York Bulls","Los Angeles Kings","Golden State Warriros","Huston Rocket"],"answer":"Huston Rocket"}},"maths":{"q1":{"question":"5 + 7 = ?","options":["10","11","12","13"],"answer":"12"},"q2":{"question":"12 - 8 = ?","options":["1","2","3","4"],"answer":"4"}}}}`),
	}

	var wg sync.WaitGroup
	for i := 0; i < 10000; i++ {
		wg.Add(1)
		go func() {
			defer wg.Done()
			key := randKey(20)
			bytesSend := dataPool[rand.Intn(len(dataPool))]
			err := cbc.Set(key, bytesSend)
			if err != nil {
				t.Error(err)
			}

			bytesRecv, _, err := cbc.Get(key)
			if err != nil {
				t.Error(err)
				return
			}

			if !bytes.Equal(bytesRecv, bytesSend) {
				t.Error("key", key, "sent and received data isn't equal")
				t.Log(string(bytesSend))
				t.Log(string(bytesRecv))
			}
		}()
	}
	wg.Wait()
	time.Sleep(2 * time.Minute)
	_ = cbc.Free()
}

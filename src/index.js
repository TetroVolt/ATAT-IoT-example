const express = require('express')
const app = express()
const port = 3000

app.get('/', (request, response) => {
  console.log("Received request: ", request);
  response.sendStatus(200);
})

app.listen(port, (err) => {
  if (err) {
    return console.error('Error! Flux capacitor broke :C ', err)
  }

  console.log(`server is listening on ${port}`)
})

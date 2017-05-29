var express = require('express')
var app = express()
var fs = require('fs')

//connect to mysql
var mysql = require('mysql')
var connection = mysql.createConnection( {
    host : '',
    user : '',
    password : '',
    database : ''
    });

connection.connect();

//To write current time

var dateTime = require('node-datetime');

//To Get IP addr
var os = require('os');
var interfaces = os.networkInterfaces();
var addresses = [];
for(var k in interfaces){
    for(var k2 in interfaces[k]){
        var address = interfaces[k][k2];
        if(address.family === 'IPv4' && !address.internal){
            addresses.push(address.address);
        }
    }
}

app.get('/',fuction(req,res) {
        //send current time and temperature to client
        if(req.query.temp && typeof req.query.temp != 'undefined'){

            var dt = dateTime.create();
            var formatted = dt.format('Y-m-d H:M:S');
            res.send(formatted+' Temp:' + req.query.temp);
            console.log(foramtted+' Temp:' + req.query.temp);

            data={};
            data.subway_id = 3000;
            data.subway_car = 1;
            data.car_temp = req.query.temp;

            //Insert data to DB by quesry
            connection.query('INSERT INTO sensor SET ?',data,function(err,rows,cols){
                        if(err) throw err;
                        console.log('Done Insert Query');
            });
         }
         else{
            res.send('Unauthorized Access');
         }
})


app.listen(8865,function() {
        console.log('Example app listening on port 8865!')
        })



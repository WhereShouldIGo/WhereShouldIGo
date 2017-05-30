var express = require('express')
var app = express()
var fs = require('fs')

//connect to mysql
var mysql = require('mysql')
var connection = mysql.createConnection( {
    host : 'localhost',
    //host : '119.192.202.112',
    user : 'root',
    password : 'alswjd005!',
    //port : '',
    database : 'wsig'
    });

//connection.connect();

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

app.get('/getdata',function(req,res) {
        //send current time and temperature to client
        //if(req.query.temp_amb && typeof req.query.temp_amb != 'undefined'){
         if(req.query.amb && req.query.obj && req.query.hum
            && typeof req.query.amb != 'undefined' && typeof req.query.obj != 'undefined' && typeof req.query.hum != 'undefined') {   
            
            var dt = dateTime.create();
            var formatted = dt.format('Y-m-d H:M:S\n');
            res.write(formatted+'temp_amb: ' + req.query.amb + '\ntemp_obj: ' + req.query.obj + '\nhum: ' + req.query.hum);
            res.end();
            console.log(formatted+'temp_amb: ' + req.query.amb + 'temp_obj: '+req.query.obj+'hum: '+req.query.hum);
/*
            res.send(formatted+' dht_hum:' + req.query.dht_hum);
            console.log(foramtted+' dht_hum:' + req.query.dht_hum);

            res.send(formatted+' dht_temp:' + req.query.dht_temp);
            console.log(foramtted+' dht_temp:' + req.query.dht_temp);

            res.send(formatted+' dht_hic:' + req.query.dht_hic);
            console.log(foramtted+' dht_hic:' + req.query.dht_hic);
*/
            data={};
            data.subway_id = 3000;
            data.subway_car = 1;
            data.temp_amb=req.query.amb;
            data.temp_obj=req.query.obj;
            data.dht_hum=req.query.hum;
            //data._temp = req.query.temp;
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

app.get('/readdata',function(req,res) {
     
       //  if(req.query.amb && req.query.obj && req.query.hum
       //      && typeof req.query.amb != 'undefined' && typeof req.query.obj != 'undefined' && typeof req.query.hum != 'undefined') {   
            connection.query('SELECT temp_obj from sensor ORDER BY data_num DESC LIMIT 1',function(err,rows,cols) {

             console.log(rows[0].temp_obj);
             data = rows[0].temp_obj;
            res.writeHeader(200,{"Content-Type" :"text/plain"});
            if(data < 26.0)
            {
                res.write("Green");
            }
            else if(data < 26.5)
            {
                res.write("Yellow");
            }
            else if(data >= 26.5) 
            {
                res.write("Red");
            }

            res.end();
        })

})
app.get('/ga',function(req,res){
    if(req.query.amb && typeof req.query.amb != 'undefined'){
        console.log(req.query.amb);
        res.send(req.query.amb);
    }
})

app.listen(8867,function() {
        console.log('Example app listening on port 8867!')
})

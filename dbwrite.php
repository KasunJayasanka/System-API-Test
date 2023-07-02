<?php

// Change port accordingly
$servername = "eu-cdbr-west-03.cleardb.net";

// REPLACE with your Database name
$dbname = "heroku_2db5e38e1987785";
// REPLACE with Database user
$username = "be4b887cdc6b20";
// REPLACE with Database user password
$password = "3ffda6fa";

// Keep this API Key value to be compatible with the ESP32 code provided in the project page.
// If you change this value, the ESP32 sketch needs to match
$api_key_value = 'tPmAT5Ab3j7F9';

// keep the respnse payload
$response = ['success' => false, 'massage' => 'Internal server error' ];
// set the http response code
$responseCode = 500; 

if ($_SERVER['REQUEST_METHOD'] == 'POST') {    
    
    // get input json payload
    $json = file_get_contents('php://input');
    $json = json_decode($json, true);

    // get api key
    $apiKey = $json['X-API-KEY'] ?? null; // shorthand operator IF X-API-KEY exist in header assing X-API-KEY value ELSE assign null


    if($apiKey == $api_key_value) {
        

        // var_dump($json); return;

        // Create connection
        $conn = new mysqli($servername, $username, $password, $dbname);
        
        // Check connection
        if ($conn->connect_error) {
            $response['massage'] = 'Connection failed'.$conn->connect_error;
            $responseCode = 500; // internal server error
        } else {
            // prepare sql connection
            // ref -> https://www.w3schools.com/php/php_mysql_prepared_statements.asp

            $stmt1 = $conn->prepare("INSERT INTO data (dateTimeStamp,gasLeakageDetected,flameDetected,temperatureValue,window1Status) VALUES (?,?,?,?,?);");

            // get values form pay load
            
            
            $dateTime = ($json['dateTime'] ?? 0);
            $gasLeakageDetected = (int)($json['gasLeakageDetected'] ?? 0);
            $flameDetected = (int)($json['flameDetected'] ?? 0); 
            $temperatureValue = (float)($json['temperatureValue'] ?? 0); 
            $window1Status = (int)($json['window1Status'] ?? 0); 
            //$window2Status = (int)($json['window2Status'] ?? 0);
            $gasWeight = (float)($json['gasWeight'] ?? 0);

            // bind paramiters to sql statement
            $stmt1->bind_param(
                'siidi',
                $dateTime, 
                $gasLeakageDetected, 
                $flameDetected, 
                $temperatureValue,
                $window1Status,
                //$window2Status,
            );

            if ($stmt1->execute() === true) {
                $response['success'] = true;
                $response['massage'] = 'Data inserted to Main Table successfully';
                $responseCode = 200; // response success
            } else {
                $response['massage'] = 'Error: ' . $sql .' '. $conn->error;
                $responseCode = 500; // internal server error
            }

            $stmt1->close();

            $stmt2 = $conn->prepare("INSERT INTO weight_data (gasWeight,dateTimeStamp) VALUES (?,?);");

            $stmt2->bind_param(
                'ds',
                $gasWeight,
                $dateTime,
            );

            if ($stmt2->execute() === true) {
                $response['success'] = true;
                $response['massage'] = 'Data inserted to Weight Table successfully';
                $responseCode = 200; // response success
            } else {
                $response['massage'] = 'Error: ' . $sql .' '. $conn->error;
                $responseCode = 500; // internal server error
            }

            $stmt2->close();
        }

        // Close the connection
        $conn->close();

    } else {
        $response['massage'] = 'Wrong API Key provided!';
        $responseCode = 401; // unauthorized request
    } 


} else {
    $response['massage'] = 'Method Not Allowed';
    $responseCode = 405; // Method Not Allowed
}

// send response to client
http_response_code($responseCode);
header('Content-Type: application/json; charset=utf-8');
echo json_encode($response);
?>

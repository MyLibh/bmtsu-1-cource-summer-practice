$client = new-object System.Net.WebClient
$client.DownloadFile("http://webrobo.mgul.ac.ru:3000/db_api_REST/calibr/log/20200622T000000/20200713T000000/Тест%20Студии/schHome", "D:/Тест студии.json")
$client.DownloadFile("http://webrobo.mgul.ac.ru:3000/db_api_REST/calibr/log/20200622T000000/20200713T000000/РОСА%20К-2/01", "D:/РОСА К-2.json")

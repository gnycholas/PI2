#1. Comando para criar a base de dados:
create schema flow_meter charset utf8mb4;

#2. Acessar a base:
use flow_meter;

#3. Criar tabela que irá receber as informações da nuvem:
create table datas(
id int auto_increment,
calculoDaVazao float (32),
diff_hour int (32),
hour datetime 
);

#4. Busca pelas informações na tabela da base:
select * from datas;

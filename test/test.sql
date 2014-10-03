drop table if exists fulltext_test;
create table fulltext_test (
	id bigint primary key auto_increment,
	text text,
	fulltext(text) with parser mysql_cn_parser
) engine=myisam default charset utf8;

insert into fulltext_test(text) values 
('其他杂环化合物'),
('其他碳水化合物'),
('其他羧酸衍生物'),
('其他非金属矿产'),
('化工管道及配件'),
('對第三丁基甲苯'),
('氰基丙烯酸甲酯'),
('流化床干燥设备'),
('滚筒刮板干燥机'),
('環六亞甲基四胺'),
('甲基丙烯酸乙酯'),
('甲基丙烯酸甲酯'),
('異丁基異氰酸鹽'),
('碳三羧基戊基錳'),
('空心桨叶干燥机'),
('苯二甲酸二丁酯'),
('苯基縮水甘油醚'),
('過氧化二苯甲醯'),
('釩亞鐵合金粉塵')

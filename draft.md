conflict betwee content and pages then do not build and show error
on templates files *.html
with syntax

<title>%content.title%</title>


if markdown file in frontmatter table dont have this meta info then fail build with error
markdown example:

---
title: Телефоны

---

# Hello

![img](/assets/images/car.png)


TODO: продумать локали (локаль по папкам файлов мд или по названию файла или в контенте во фронтматтере) и конфликты и возможно есть локаль в страницах но нет в контенте
мб сделать два режима без локали и с локалью

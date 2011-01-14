﻿m4_include(`version.m4')m4_dnl
.Language=Russian,Russian (Русский)
.PluginContents=Observer

@Contents
`$^#Observer' OBSERVER_VERSION`#'

 Этот плагин позволяет просмотривать файлы для контейнеров различных типов.
 Все поддерживаемые форматы допускают просмотр списков файлов и их извлечение.

 Список поддерживаемых форматов:
 - Установочные пакеты #MSI#
 - Установочные пакеты #NSIS#
 - Образы #UDF# (ISO 13346)
 - Образы CD дисков : #ISO-9660# (вкл. Joliet), #CUE/BIN#, #MDF/MDS#
 - Файлы Volition Pack V2 (из игр Freespace 1/2/Open)
 - Egosoft packages (for X-series games)
 - Контейнеры MIME
 - Почтовые базы MS Outlook
 - Установочные пакеты #WISE#

 (c) 2009-2010, Ariman
 -----------------------------------------
 Web-site: ~http://ariman.creiac.com~@http://ariman.creiac.com@
 Контактный E-mail: ~ariman@@inbox.ru~@mailto:ariman@@inbox.ru@

@ObserverConfig
$ #Конфигурация Observer-а#
    В этом диалоге можно изменять следующие параметры:

 #Enable plugin#
 Включает / Отключает реакцию плагина на Enter и Ctrl-PgDn.
 Эта опция не влияет на меню плагинов (F11).

 #Use prefix#
 Включает / Отключает использование префикса плагина.
 
 #Prefix value#
 Задает значение префикса плагина (32 символа максимум).
 
@ObserverExtract
$ #Извлечение файлов#
    В данном диалоге можно настраивать различные параметры извлечения файлов.

 #Overwrite existing files#
 Задает поведение для перезаписи существующих файлов.
 [ ] - пропускать файлы которые уже существуют,
 [x] - перезаписывать существующие файлы,
 [?] - спрашивать (значение по умолчанию). 

 #Keep paths#
 Задает поведение плагина при формировании путей для файлов.
 [ ] - извлечь все файлы в существующий каталог,
 [x] - сохранять полные пути файлов в контейнере начиная с корня,
 [?] - сохранять пути файлов в контейнере начиная с текущего каталога (значение по умолчанию).

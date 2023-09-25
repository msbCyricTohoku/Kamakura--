#include "highlighter.h"

#include <QDebug>
//Kamakura-- Mehrdad S. Beni and Hiroshi Watabe, Japan 2023
Highlighter::Highlighter(const QString _filename, QObject *parent) : QSyntaxHighlighter(parent) {
    xml_filename=_filename;
    QFile file(xml_filename);
    QStringList keywords;
    QStringList params;
   // QStringList extraparams;
  //  QStringList matparams;
    if (file.open(QIODevice::ReadOnly)) {
        HighlightFormat my_format;
        HighlightFormat my_format2;
        QDomDocument doc;
        doc.setContent(&file);
        auto languages = doc.elementsByTagName("language");
        for (int i = 0; i < languages.length(); ++i) {
            auto ext = languages.at(i).toElement();

            auto file_extension = QRegExp(ext.attribute("extentions"));

            HighlightLang data = {};

            QDomNodeList list = ext.elementsByTagName("keyword");
            int size = list.size();
            for (int i = 0; i < size; ++i){
                keywords.append(list.at(i).toElement().text());
            }

            data.keyword_format.setForeground(Qt::darkBlue);
            data.keyword_format.setFontWeight(QFont::Bold);
            foreach (const QString& _pattern, keywords) {

                my_format.pattern = QRegExp("\\b"+_pattern+"\\b");
                my_format.format = data.keyword_format;
                data.for_keywords.append(my_format);
            }
            keywords.clear();
            list = ext.elementsByTagName("command");
            size = list.size();
            data.preproc_format.setForeground(Qt::darkBlue);
            for (int i = 0; i < size; ++i)
                keywords.append(list.at(i).toElement().text());

            foreach(const QString& _pattern, keywords){
                my_format.pattern = QRegExp(_pattern);
                my_format.format = data.preproc_format;
                data.for_keywords.append(my_format);
            }
//***********************************************************************

            QDomNodeList list2 = ext.elementsByTagName("param");
            int size2 = list2.size();
            for (int i = 0; i < size2; ++i){
                keywords.append(list2.at(i).toElement().text());
            }

            data.keyword_format.setForeground(Qt::blue);
            data.keyword_format.setFontWeight(QFont::DemiBold);
            foreach (const QString& _pattern, keywords) {

                my_format.pattern = QRegExp("\\b"+_pattern+"\\b");
                my_format.format = data.keyword_format;
                data.for_keywords.append(my_format);
            }
            keywords.clear();

//***********************************************************************

            QDomNodeList list3 = ext.elementsByTagName("extraparam");
            int size3 = list3.size();
            for (int i = 0; i < size3; ++i){
                keywords.append(list3.at(i).toElement().text());
            }

            data.keyword_format.setForeground(Qt::red);
            data.keyword_format.setFontWeight(QFont::ExtraExpanded);
            foreach (const QString& _pattern, keywords) {

                my_format.pattern = QRegExp("\\b"+_pattern+"\\b");
                my_format.format = data.keyword_format;
                data.for_keywords.append(my_format);
            }
            keywords.clear();

            QDomNodeList list4 = ext.elementsByTagName("matparam");
            int size4 = list4.size();
            for (int i = 0; i < size3; ++i){
                keywords.append(list4.at(i).toElement().text());
            }

            data.keyword_format.setForeground(Qt::green);
            data.keyword_format.setFontWeight(QFont::Bold);
            foreach (const QString& _pattern, keywords) {

                my_format.pattern = QRegExp("\\b"+_pattern+"\\b");
                my_format.format = data.keyword_format;
                data.for_keywords.append(my_format);
            }
            keywords.clear();

/*
            QDomNodeList list3 = ext.elementsByTagName("extraparam");
            int size3 = list3.size();
            for (int i = 0; i < size3; ++i){
                extraparams.append(list3.at(i).toElement().text());
            }

            QDomNodeList list4 = ext.elementsByTagName("matparam");
            int size4 = list4.size();
            for (int i = 0; i < size4; ++i){
                matparams.append(list4.at(i).toElement().text());
            }

*/


            data.classname_format.setForeground(Qt::darkMagenta);
            data.classname_format.setFontWeight(QFont::Bold);
            my_format.pattern = QRegExp("\\bQ[A-Za-z]+\\b");
            my_format.format = data.classname_format;
            data.for_keywords.append(my_format);
            data.multi_comment.setForeground(Qt::green);
            QDomNode beg_com = ext.elementsByTagName("multiline_comment").at(0);
            data.comment_beg_exp=QRegExp(beg_com.toElement().attribute("begin"));
            data.comment_end_exp=QRegExp(beg_com.toElement().attribute("end"));

            data.single_comment2.setForeground(QColor(Qt::green).lighter(50));
            QDomNode single_com2 = ext.elementsByTagName("single_comment2").at(0);
            my_format.pattern = QRegExp(single_com2.toElement().attribute("expression2"));
            my_format.format = data.single_comment2;
            data.for_keywords.append(my_format);

            data.single_comment.setForeground(QColor(Qt::green).lighter(50));
            QDomNode single_com = ext.elementsByTagName("single_comment").at(0);
            my_format.pattern = QRegExp(single_com.toElement().attribute("expression"));
            my_format.format = data.single_comment;
            data.for_keywords.append(my_format);


            data.quotation_format.setForeground(Qt::green);
            my_format.pattern = QRegExp("\".*\"");
            my_format.format = data.quotation_format;
            data.for_keywords.append(my_format);

            data.function_format.setFontItalic(false);
            data.function_format.setForeground(Qt::magenta);
          //  my_format.pattern = QRegExp("[A-Za-z0-9_]+[?=\\()");
            my_format.pattern = QRegExp("=");
            my_format.format = data.function_format;
            data.for_keywords.append(my_format);

            langs.insert(file_extension, data);


        }
    }
}

void Highlighter::highlightBlock(const QString& text) {
    auto &data = langs.value(current_extension);


    foreach (const HighlightFormat& rule_, data.for_keywords) {
        QRegExp expression(rule_.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int len = expression.matchedLength();
            setFormat(index, len, rule_.format);
            index = expression.indexIn(text, index+len);
        }
    }
    setCurrentBlockState(0);
    int begin = 0;
    if (previousBlockState() != 1)
        begin = data.comment_beg_exp.indexIn(text);

    while (begin >= 1) {
        int end = data.comment_end_exp.indexIn(text);
        int com_len;
        if (end == -1) {
            setCurrentBlockState(1);
            com_len = text.length() - begin + data.comment_end_exp.matchedLength();
        }
        else {
            com_len = end - begin + data.comment_end_exp.matchedLength();
        }
        setFormat(begin, com_len, data.multi_comment);
        begin = data.comment_beg_exp.indexIn(text, begin + com_len);
    }
}

bool Highlighter::setExtension(const QString &_filename) {
    for (int i = 0; i < langs.size(); ++i) {
        if (langs.keys().at(i).indexIn(_filename) != -1){
            current_extension = langs.keys().at(i);
            return true;
        }
    }

    return false;
}
//Kamakura-- Mehrdad S. Beni and Hiroshi Watabe, Japan 2023

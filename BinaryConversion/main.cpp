#include <QCoreApplication>
#include <QTextStream>
#include <QString>

QString toBinary(int value) {
    QString result;
    for (int i = sizeof(value) * 8 - 1; i >= 0; --i) {
        int bit = (value >> i) & 1;
        if (!result.isEmpty() || bit) {
            result.append(QString::number(bit));
        }
    }
    return result.isEmpty() ? "0" : result;
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    QTextStream in(stdin);
    QTextStream out(stdout);

    out << "Enter an octal number: ";
    out.flush();

    QString input = in.readLine();
    bool ok;
    int oct = input.toInt(&ok, 8); // interpret as base-8

    if (!ok) {
        out << "Invalid octal input.\n";
        return 1;
    }

    out << "Decimal    : " << oct << "\n";
    out << "Hexadecimal: " << QString::number(oct, 16).toUpper() << "\n";
    out << "Binary     : " << toBinary(oct) << "\n";

    return 0;
}

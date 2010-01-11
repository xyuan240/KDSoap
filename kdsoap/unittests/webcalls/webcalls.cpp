#include "KDSoapClientInterface.h"
#include "KDSoapMessage.h"
#include "KDSoapValue.h"
#include "KDSoapPendingCallWatcher.h"
#include "soapresponder.h"
#include <QtTest/QtTest>
#include <QEventLoop>
#include <QDebug>

class TestObject : public QObject
{
    Q_OBJECT
public:

public slots:
    void slotFinished(KDSoapPendingCallWatcher* watcher)
    {
        m_returnArguments = watcher->returnArguments();
        m_returnValue = watcher->returnValue();
        m_eventLoop.quit();
    }

private slots:

    void testAddIntegers_async()
    {
        const QString endPoint = QString::fromLatin1("http://www.mathertel.de/AJAXEngine/S02_AJAXCoreSamples/CalcService.asmx");
        const QString messageNamespace = QString::fromLatin1("http://www.mathertel.de/CalcFactors/");
        KDSoapClientInterface client(endPoint, messageNamespace);
        KDSoapMessage message;
        message.addArgument(QLatin1String("number1"), 42);
        message.addArgument(QLatin1String("number2"), 43);
        KDSoapPendingCall pendingCall = client.asyncCall("AddInteger", message/*, action*/);
        KDSoapPendingCallWatcher *watcher = new KDSoapPendingCallWatcher(pendingCall, this);
        connect(watcher, SIGNAL(finished(KDSoapPendingCallWatcher*)),
                this, SLOT(slotFinished(KDSoapPendingCallWatcher*)));
        m_eventLoop.exec();
        QCOMPARE(m_returnValue.toInt(), 85);
    }

    void testAddIntegers_sync()
    {
        const QString endPoint = QString::fromLatin1("http://www.mathertel.de/AJAXEngine/S02_AJAXCoreSamples/CalcService.asmx");
        const QString messageNamespace = QString::fromLatin1("http://www.mathertel.de/CalcFactors/");
        KDSoapClientInterface client(endPoint, messageNamespace);
        KDSoapMessage message;
        message.addArgument(QLatin1String("number1"), 42);
        message.addArgument(QLatin1String("number2"), 43);
        KDSoapMessage ret = client.call("AddInteger", message);
        QCOMPARE(ret.arguments().first().value().toInt(), 85);
    }

    void testHolidays()
    {
        const int year = 2009;
        const QString endPoint = QString::fromLatin1("http://www.27seconds.com/Holidays/US/Dates/USHolidayDates.asmx");
        const QString messageNamespace = QString::fromLatin1("http://www.27seconds.com/Holidays/US/Dates/");
        KDSoapClientInterface client(endPoint, messageNamespace);
        KDSoapMessage message;
        message.addArgument(QLatin1String("year"), year);
        KDSoapPendingCall pendingCall = client.asyncCall("GetValentinesDay", message/*, action*/);
        KDSoapPendingCallWatcher *watcher = new KDSoapPendingCallWatcher(pendingCall, this);
        connect(watcher, SIGNAL(finished(KDSoapPendingCallWatcher*)),
                this, SLOT(slotFinished(KDSoapPendingCallWatcher*)));
        m_eventLoop.exec();
        // TODO how are we supposed to know / tell that it's a datetime?
        QCOMPARE(m_returnValue, QVariant("2009-02-14T00:00:00.0000000-05:00"));
    }

    //  http://www.soapclient.com/soapclient?fn=soapform&template=/clientform.html&soaptemplate=/soapresult.html&soapwsdl=http://soapclient.com/xml/soapresponder.wsdl

    void testSoapResponder_sync()
    {
        SoapResponder responder;
        QString ret = responder.Method1("abc", "def");
        QCOMPARE(ret, QString("Your input parameters are abc and def"));
    }

    void testSoapResponder_async()
    {
        SoapResponder responder;
        QSignalSpy spyDone(&responder, SIGNAL(Method1Done(QString)));
        connect(&responder, SIGNAL(Method1Done(QString)), &m_eventLoop, SLOT(quit()));
        responder.asyncMethod1("abc", "def");
        m_eventLoop.exec();
        QCOMPARE(spyDone.count(), 1);
        QCOMPARE(spyDone[0][0].toString(), QString("Your input parameters are abc and def"));
    }

    void testFault()
    {
        const QString endPoint = QString::fromLatin1("http://soapclient.com/xml/doesnotexist");
        const QString messageNamespace = QString::fromLatin1("incorrect, just for testing");
        KDSoapClientInterface client(endPoint, messageNamespace);
        KDSoapMessage message;
        message.addArgument(QLatin1String("bstrParam1"), QLatin1String("abc"));
        message.addArgument(QLatin1String("bstrParam2"), QLatin1String("def"));
        KDSoapMessage ret = client.call("Method1", message);
        qDebug() << ret;
        QVERIFY(ret.isFault());
        QCOMPARE(ret.faultAsString(), QString::fromLatin1("Fault code: SOAP-ENV:Server\nFault description: The parameter is incorrect. (/xml/doesnotexist)"));
    }

    // http://www.service-repository.com/service/wsdl?id=163859
    void testServiceRepositoryCom()
    {
        const QString endPoint = QString::fromLatin1("http://www.thomas-bayer.com/names-service/soap");
        const QString messageNamespace = QString::fromLatin1("http://namesservice.thomas_bayer.com/");
        const QString action = QString::fromLatin1("");
        KDSoapClientInterface client(endPoint, messageNamespace);
        KDSoapMessage message;
        KDSoapPendingCall pendingCall = client.asyncCall("getCountries", message, action);
        KDSoapPendingCallWatcher *watcher = new KDSoapPendingCallWatcher(pendingCall, this);
        connect(watcher, SIGNAL(finished(KDSoapPendingCallWatcher*)),
                this, SLOT(slotFinished(KDSoapPendingCallWatcher*)));
        m_eventLoop.exec();
        qDebug() << m_returnValue;
        qDebug() << m_returnArguments;

        // TODO QCOMPARE(m_returnArguments[0], QString::fromLatin1("Great Britain"));
    }

    // TODO: a great example for complex returned structures:
    // http://www.holidaywebservice.com/Holidays/HolidayService.asmx?op=GetHolidaysForYear

private:
    QEventLoop m_eventLoop;
    QVariant m_returnValue;
    KDSoapMessage m_returnArguments;
};

QTEST_MAIN(TestObject)

#include "webcalls.moc"
#include <QtCore/QTimer>
#include <QtCore/QUrl>

#include "apoolmonitor.h"
#include "apoolchecker.h"
#include "alogger.h"

// ========================================================================== //
// Конструктор.
// ========================================================================== //
APoolMonitor::APoolMonitor(QObject *parent)
    : QObject(parent), _has_pool_changed(false), _port(3337)
    , _pool_checker(new APoolChecker(this)), _timer(new QTimer(this)) {

    connect(_pool_checker, SIGNAL(succeed()), this, SIGNAL(succeed()));
    connect(_pool_checker, SIGNAL(failed()), this, SLOT(stop()));
    connect(_pool_checker, SIGNAL(failed()), this, SIGNAL(failed()));

    _timer->setInterval(5000);
    _timer->setSingleShot(true);

    connect(_pool_checker, SIGNAL(succeed()), _timer, SLOT(start()));
    connect(_timer, SIGNAL(timeout()), this, SLOT(onTimerTimeout()));
}


// ========================================================================== //
// Слот установки пула на мониторинг.
// ========================================================================== //
void APoolMonitor::changePool(const QString &pool) {
    QUrl url = QUrl::fromUserInput(pool);
    if(!url.isEmpty()) {
        QString host = url.host();
        int     port = url.port();
        if(!host.isEmpty() && port != -1) {
            logInfo(QString("pool monitoring %1:%2 started")
                .arg(host).arg(port));

            _has_pool_changed = true; _host = host; _port = port;

            return _pool_checker->check(host, port);
        }
    }

    emit failed();
}


// ========================================================================== //
// Слот деактивации мониторинга.
// ========================================================================== //
void APoolMonitor::stop() {_timer->stop(); _host.clear(); _port = -1;}


// ========================================================================== //
// Слот активации таймера.
// ========================================================================== //
void APoolMonitor::onTimerTimeout() {
    _has_pool_changed = false; _pool_checker->check(_host, _port);
}
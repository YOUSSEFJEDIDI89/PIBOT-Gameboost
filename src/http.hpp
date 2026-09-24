// http.hpp - طبقة HTTP: libcurl إن وُجد وقت البناء، وإلا أمر curl تلقائياً
#pragma once
#include <string>
#include <vector>

// POST مع جسم JSON. ترجع جسم الرد (فارغ عند فشل الاتصال)
std::string httpPostJson(const std::string& url,
                         const std::vector<std::string>& headers,
                         const std::string& body,
                         long timeoutSec = 90);

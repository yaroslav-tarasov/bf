#include "utils.h"

#include <QDateTime>
#include <QFile>
#include <QDebug>

#include <sys/sysinfo.h>

namespace sys_utils
{


    bool isReboot(const QString& bootTimeFile)
    {
         static time_t  boottime;
         struct sysinfo mysysinfo;

         memset( &mysysinfo,0,sizeof(mysysinfo));
         sysinfo(&mysysinfo);

         boottime = time(0)- mysysinfo.uptime;

         qDebug() << "isReboot:Bootime:" <<QDateTime::fromTime_t(boottime) ;
         QFile file(bootTimeFile);

         if(!file.exists())
         {
              file.open( QIODevice::WriteOnly ) ;
              file.write ( (char *)&boottime, sizeof(time_t) ) ;
              file.close();
              qDebug() << "isReboot:true" ;
              return   true;
         }

         time_t oldbootime;
         file.open( QIODevice::ReadWrite ) ;
         file.read( (char *)&oldbootime, sizeof(time_t) ) ;

         if((  boottime - oldbootime ) >1 )
         {
              file.seek(0);
              file.write ( (char *)&boottime, sizeof(time_t) ) ;
              file.close();
              qDebug() << "isReboot:true:" << "oldbootime:" <<oldbootime<<
              " boottime:" << boottime;
              return  true;
         }

         qDebug() << "isReboot:false" ;
         return   false;
    }

}

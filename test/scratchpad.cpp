
void dummyTask(void *b)
{
    while(1)
    {
        xDelay(100);
    }
}

float scanCoil( TestPin &pin)
{
    float L;
    int nbSamples;
    uint16_t *samples;
    
    // Maximum speed
    if(!pin.prepareDmaSample(  ADC_SMPR_1_5,   DSOADC::ADC_PRESCALER_6, 1024)) 
        xAssert(0);
    if(!pin.finishDmaSample(nbSamples,&samples)) 
    {
        xAssert(0);
    }
    // Search the maximum, that gives us the peak
    int mx=0;
    for(int i=0;i<nbSamples;i++)
        if(samples[i]>mx) mx=samples[i];

    if(mx<10) // too small, we are in the noise
        return 0;

    // mx is the ratio between the resistor and the coil 
    float z1=pin.getCurrentRes();
    float den=4095./(float)mx;
    den-=1.;

    float z2=z1/den;
    //
    z2-=pin.getRes(TestPin::VCC); // take parasitic resistance into account
    if(z2<0) return 0;
    // Z2=jWL = j * 2*PI*F*L => L=Z2/2Pif
    L=z2/(2.*M_PI);
    // divide by frequency
    L/=51000.;
    return L;
}


#if 0
 probeCap(pin2,pin1);
#endif
    
#if 0     
    pin1.setMode(TestPin::PULLUP_PWM);
    pin2.setMode(TestPin::GND);
    xDelay(10); // let it stabilize

    while(1)
    {
        float coil=0;
        for(int i=0;i<8;i++)
            coil+=scanCoil(pin1);
        coil/=8.;
        
        char outPrint[100];
        Component::prettyPrint(coil,"H",outPrint);
        TesterGfx::clear();
         TesterGfx::print(10,60,outPrint);
         xDelay(200);
        
        
    }
#endif    
    
#if 0    
    // 
     pin2.setToGround();
     pin1.pwm(TestPin::PULL_LOW,1000);
     while(1)
     {
         
     }
#endif
    
#if 0
    //TesterGfx::drawDiode(0,"5pf",1,2);
    //TesterGfx::drawResistor(0,"5kO",1,2);
    //TesterGfx::drawCapacitor(0,"5pF",1,2);
    TesterGfx::drawCoil(0,"5pF",1,2);
    
    
    while(1)
    {
        
    };
#endif    
    
#if 0  
    TesterControl::test();
#endif
#if 0
    TesterGfx::test();
#endif    
#if 0    
    pinTest();
    while(1)
    {
        
    };
#endif
    
    
    
/**
 * \fn probe for 3 poles 
 * @param a
 * @param b
 * @param c
 * @param comp
 */
bool probeMe3(TestPin &a,TestPin &b,TestPin &c,Component **comp)
{
    COMPONENT_TYPE xtype;
    Component *c2=Component::identity3(a,b,c,xtype);
    if(!c2)
        return false;
  
    delete *comp;
    *comp=c2;
    return true;
}
/**
 * \fn probe for 2 poles
 * @param a
 * @param b
 * @param c
 * @param comp
 */
void probeMe2(TestPin &a,TestPin &b,TestPin &c,Component **comp)
{
    COMPONENT_TYPE xtype;
    Component *c2=Component::identity2(a,b,c,xtype);
    if(!c2)
        return;
    
    if(!*comp) 
    {
        *comp=c2;
        return ;
    }        
    bool replace=false;
    if(c2->likely()>(*comp)->likely())
    {
        replace=true;
    }
    if(replace)
    {
        delete *comp;
        *comp=c2;
    }else
    {
        delete c2;
    } 
}

  COMPONENT_TYPE type;
    
    // 1 : Zeroing
    TesterGfx::clear();
    TesterGfx::print(30,40,"Zeroing..");
    zeroAllPins();
    TesterGfx::clear();
    TesterGfx::print(0,40,"Click to probe");
next:
    TesterGfx::clear();
    TesterGfx::print(0,40,"Detecting");
    
#if 1      
    Component *c=NULL;

#define PROBEME3(column, alpha, beta,gamma)     {TesterGfx::print(column,75,"x");   probeMe3(alpha,beta,gamma,&c);}     
#define PROBEME(column, alpha, beta,gamma)     {TesterGfx::print(column,75,"*");    probeMe2(alpha,beta,gamma,&c);} 
    
    PROBEME3(20,pin1,pin2,pin3)
    PROBEME3(40,pin1,pin3,pin2)
    PROBEME3(60,pin2,pin3,pin1)
    
    if(!c)
    {
        PROBEME(20,pin1,pin2,pin3)
        PROBEME(40,pin1,pin3,pin2)
        PROBEME(60,pin2,pin3,pin1)
        
    }
    
    
#else
    TesterGfx::printStatus("Probing");
    Component *c=new NPNBjt(pin2,pin3,pin1);
    //Component *c=new Coil(pin1,pin2,pin3);
    //Component *c=new Capacitor(pin3,pin2,pin1);
#endif
    if(!c)
    {
        TesterGfx::clear();
        TesterGfx::print(0,60,"Found nothing");
        if(TesterControl::waitForEvent() & CONTROL_LONG)
            menuSystem();
        return;
    }
    TesterGfx::clear();
    const char *sname=c->getShortName();
    TesterGfx::print(8,40,sname);
    TesterGfx::print(0,60,"  Measuring");

    // Valid component detected ?
    if(c->compute())
    {   
        zeroAllPins();
        TesterGfx::clear();
        c->draw(0);     
        while(1)
        {
            int evt=TesterControl::waitForEvent();            
            if(evt & CONTROL_LONG) menuSystem(); // probe next
            if(evt & CONTROL_SHORT) goto next; // probe next
            // TODO LONG => menu
            if(evt & CONTROL_ROTARY)
            {
                int count=TesterControl::getRotary();
                c->changePage(count);
            }
        }

        
    }
    delete c;
    TesterGfx::printStatus("-------------");
    for(int i=0;i<5;i++)
    {
        digitalWrite(LED,HIGH);
        xDelay(200);
        digitalWrite(LED,LOW);
        xDelay(200);
    }
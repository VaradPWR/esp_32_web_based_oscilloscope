#ifndef FREQUENCY_CORRECTOR_H
#define FREQUENCY_CORRECTOR_H

extern float CorrectFrequency;

void initFrequencyCorrector();       // Call this in setup()
void updateCorrectFrequencyLoop();   // Call this in loop()

#endif

///#ifndef FREQUENCY_CORRECTOR_H
//#define FREQUENCY_CORRECTOR_H

//void initFrequencyCorrector();
//void updateCorrectFrequencyLoop();
//float getCorrectedFrequency();

//#endif

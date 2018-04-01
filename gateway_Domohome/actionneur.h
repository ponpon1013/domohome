#ifndef _ACTIONNEUR_H
  #define _ACTIONNEUR_H


class actionneur{
  public:
    actionneur();
    void setNumeroPin(int);
    void setNumeroNoeud(int);
    int getNumeroNoeud();
    int getNumeroPin();
    bool state;
   private:
    int numeroPin;
    int numeroNoeud;
};


#endif

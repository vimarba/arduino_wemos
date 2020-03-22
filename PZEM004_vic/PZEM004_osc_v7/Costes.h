

#define PotContratada   3
#define EurosKWdia      0.1232288
#define EurosKWhora     0.149877
#define Descuento       0.10
#define ImpuestoElectr  0.051127
#define Alquiler        0.02663
#define IVA             0.21

float PotenciaFacturada,EnergiaFacturada,DescuentoConsumo,ImpuestoElectricidad,TotalEnergia,AlquilerEquipos,TotalImporte;


float CalculoCoste(float Consumo,int dias)
{
  
  PotenciaFacturada=PotContratada*dias*EurosKWdia;
  EnergiaFacturada=Consumo*EurosKWhora;
  DescuentoConsumo=Descuento*EnergiaFacturada;
  ImpuestoElectricidad=ImpuestoElectr*(PotenciaFacturada+EnergiaFacturada);

  TotalEnergia=EnergiaFacturada+ImpuestoElectricidad+PotenciaFacturada-DescuentoConsumo;

  AlquilerEquipos=dias*Alquiler;

  TotalImporte=(1+IVA)*(TotalEnergia+AlquilerEquipos);

  return TotalImporte;
  }

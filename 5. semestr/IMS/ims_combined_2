double Time = 0, st[3];

void Event() {
  LOG(time, st[0], st[1], st[2]); 
  Cal_Insert(Event, Time + Uniform(1, 5)); }
  
void Dynamic(double in[]) {
  in[0] = 2 * st[0] / 3; 
  in[1] = -4 * st[1] - 5 * in[0]; 
  in[2] = st[1]; }
  
void Euler_step(double step) {
  double in[3];
  Dynamic(st, in);
  for (int i = 0; i < 3; i++)
    st[i] += step * in[i];
  Time += step; }
  
int main() {
  double step = 1.0/16;
  Cal_Init(); Cal_Insert(END_EVENT, 200); Cal_Insert(Event, Time + Uniform(1, 5));
  struct event e;
  while (1) {
    while (Time < e.time) {
      if ((Time + step * 1.01) > e.time)
        Euler_step(e.time - Time);
      else
      	Euler_step(step);
    }
       
    Time = e.time; e.Behavior();
    if (e.event_ptr == END_EVENT) break;
  } 
}

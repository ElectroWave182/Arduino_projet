#include <DFRobot_RGBMatrix.h> // Hardware-specific library

#define OE     9
#define LAT   10
#define CLK   11
#define A     A0
#define B     A1
#define C     A2
#define D     A3
#define E     A4
#define WIDTH 64
#define HEIGHT 64

DFRobot_RGBMatrix matrix (A, B, C, D, E, CLK, LAT, OE, false, WIDTH, HEIGHT);


int etat =1;
/*
Etat 1 = Ecran de debut du jeu
Etat 2 = Jeu en cours
Etat 3 = Pause
Etat 4 = Ecran de mort
*/
int bouton = 7;
int score;
int const POSITION_SOCLE = 10;
int TAILLE_JOUEUR = 7;
long last_clicked = 0;
int last_bouton_state = 1;

int couleur[] = {
  matrix.Color333 (0, 0, 0), // noir
  matrix.Color333 (6, 6, 6), // gris clair
  matrix.Color333 (7, 0, 0), // rouge
  matrix.Color333 (0, 2, 7), // bleu
  matrix.Color333 (0, 7, 0), // vert clair
  matrix.Color333 (3, 3, 3), // gris
  matrix.Color333 (7, 4, 0), // jaune
  matrix.Color333 (7, 2, 0), // orange
  matrix.Color333 (2, 0, 4)  // violet
};


void effacePixel (int colonne, int ligne) {
  matrix.drawPixel (colonne, ligne, couleur[0]);
}

void dessinePixelTriangle (int colonne, int ligne) {
  matrix.drawPixel (colonne, ligne, couleur[2]);
}

void dessinePixelSol (int colonne, int ligne) {
  matrix.drawPixel (colonne, ligne, couleur[1]);
}

void reset () {
  matrix.fillScreen (couleur[0]);
}

void draw_start () {
  reset ();
  matrix.setTextSize(2);
  matrix.setCursor(3, 20);
  matrix.setTextColor(couleur[6]);
  matrix.println("START");
  matrix.fillRect(3,37, 58, 2, couleur[6]);
}

void draw_pause () {
  matrix.setTextSize(1);
  matrix.setCursor(1,1);
  matrix.setTextColor(couleur[3]);
  matrix.println("PAUSE");
}

void draw_socle () {
  matrix.fillRect (0, 0, WIDTH, POSITION_SOCLE, couleur[1]);
  matrix.fillRect (0, HEIGHT - POSITION_SOCLE, WIDTH, POSITION_SOCLE, couleur[1]);
}


class Triangle {

  public:
    int x, y, taille, env;
    bool retourne;
    Triangle (int x1, int y1, int t, bool ret) {
      x = x1;
      y = y1;
      taille = t;
      env = 2 * taille + 1;
      retourne = ret;
    }

    Triangle () {}

    void make_move_endroit () {

      for (int colonne = 0; colonne < env; colonne ++) {
        for (int ligne = 0; ligne < env; ligne ++) {
          if (colonne - taille == ligne / 2) {
            effacePixel (colonne + x, ligne + y - env+1);
          }
        }
      }

      x -= 1;

      for (int colonne = 0; colonne < env; colonne ++) {
        for (int ligne = 0; ligne < env; ligne ++) {
          if (taille - colonne == ligne / 2) {
            dessinePixelTriangle (colonne + x, ligne + y - env+1);
          }
        }
      }

    }

    void make_move_envers () {

      for (int colonne = 0; colonne < env; colonne ++) {
        for (int ligne = 0; ligne < env; ligne ++) {
          if (env - colonne - 1 == (ligne + 1) / 2) {
            effacePixel (colonne + x, ligne + y);
          }
        }
      }

      x -= 1;

      for (int colonne = 0; colonne < env; colonne ++) {
        for (int ligne = 0; ligne < env; ligne ++) {
          if (colonne == (ligne + 1) / 2) {
            dessinePixelTriangle (colonne + x, ligne + y);
          }
        }
      }

    }

    void make_move () {
      if (retourne) {
        make_move_envers ();
      }
      else {
        make_move_endroit ();
      }
    }
};

Triangle triangles[10] = {};


class Player {
  
  public:
    int x, y, numCouleur, taille, y_velocity, y_acceleration, amplitude_saut;
    bool is_moving;
    String nom;
    Player (String n, int x1, int y1, int num) {
      x = x1;
      y = y1;
      numCouleur = num;
      taille = TAILLE_JOUEUR;
      y_velocity = 0;
      y_acceleration = 1;
      amplitude_saut = 8;
      is_moving = false;
      nom=n;
      
    }

    void draw () {
      matrix.fillRect (x, y - taille+1, taille, taille, couleur[numCouleur]);
    }

    void draw_black () {
      matrix.fillRect (x, y - taille+1, taille, taille, couleur[0]);
    }

    void make_move () {
      if (y_velocity > 0) {
        y -= y_velocity;
      }
      else {
        y -= y_velocity / 3;
      }
      
      y_velocity -= y_acceleration;
      
      if (y > HEIGHT - POSITION_SOCLE-1) {
        y = HEIGHT - POSITION_SOCLE-1;
        is_moving = false;
      }
    }

    void jump () {
        is_moving = true;
        y_velocity = amplitude_saut;
    }

    bool saut_possible () { // A changer si on veut mettre des plateformes en hauteur
      return y == POSITION_SOCLE;
    }

    bool touche () {
        Triangle tri;
        for (int numTriangle = 0; numTriangle < 10; numTriangle ++) {
            tri = triangles[numTriangle];
            
            if (0 <= tri.x && tri.x < x + taille) {
                for (int colonne = 0; colonne < tri.env; colonne ++) {
                    for (int ligne = 0; ligne < tri.env; ligne ++) {
                        if (tri.retourne) {
                            
                            // selection au triangle
                            if (
                            tri.env - colonne - 1 == (ligne + 1) / 2
                            || colonne == (ligne + 1) / 2
                            ) {
                                
                                // selection au carre
                                if (
                                x <= colonne + tri.x && colonne + tri.x < x + taille
                                && (ligne + tri.y == y || ligne + tri.y == y - taille +1)
                                ) {
                                
                                    return true;
                                }
                            }
                        }
                        else {
                            // selection au triangle
                            if (abs (colonne - tri.taille) == ligne / 2) {
                                
                                // selection au carre
                                if (
                                x <= colonne + tri.x && colonne + tri.x< x + taille
                                && (ligne + tri.y - tri.env +1 == y || ligne + tri.y - tri.env +1 == y - taille +1)
                                ) {
                                
                                    return true;
                                }
                            }
                        }
                    }
                }
            }
        }
            

      return false;
    }

};


Player my_player = Player("Nicolas", 6, HEIGHT - POSITION_SOCLE -1, 5);

void draw_deathScreen() {
  reset ();
  matrix.setTextSize(1);
  matrix.setCursor(6, 6);
  matrix.setTextColor(couleur[2]);
  matrix.println("GAME OVER");

  matrix.setCursor(6, 24);
  matrix.setTextColor(couleur[1]);
  matrix.print(my_player.nom);
  matrix.setCursor(6, 42);
  matrix.println(score);
}

void init_triangle () {
  int current_x = 40;
  int i=0;
  int taille_iteration = 10;
  int le_type;
  Serial.println("start");
  while (i < taille_iteration){
    le_type = random(6);
    switch (le_type) {
      
      case 2: // triangle double au sol
        current_x += random (20) + 20;

        long taille = random(3) +1;
        triangles[i] = Triangle (current_x, HEIGHT - POSITION_SOCLE-1, taille, false);
        i ++;
        if(i==10) {return;}
        current_x += taille*2+1;
        triangles[i] = Triangle (current_x, HEIGHT - POSITION_SOCLE-1, taille, false);
        current_x += taille*2+1;
        i++;
        if(i==10) {return;}
        
      case 1: // triangles au plafond
        current_x += random (20) + 20;
        int nb_triangle = random (5) + 1;
        for (int j = 0; j < nb_triangle; j ++) {
          long taille = random(3) +1;
          triangles[i] = Triangle (current_x, POSITION_SOCLE, taille, true);
          current_x += taille*2+1;
          i++;
          if(i==10) {return;}
        }
        

      default:
        current_x += random (40)+ 20;
        taille = random(3) +1;
        triangles[i] = Triangle (current_x, HEIGHT - POSITION_SOCLE-1, taille, false);
        current_x += 2*taille+1;
        i++;
        if(i==10) {return;}
        
    }
  }
}

bool game_lost () {
  for (int i = 0; i < 20; i ++) {
    if (triangles[i].x <= my_player.x + 6
      && triangles[i].x + 7 > my_player.x + 6 && ! my_player.is_moving) {
        return true;
    }
    if (my_player.x < triangles[i].x + 7
      && my_player.x + 6 > triangles[i].x
      && ! my_player.is_moving) {
        return true;
    }
  }

  return false;
}


void setup () {
  randomSeed(analogRead(8));
  matrix.begin ();
  Serial.begin (9600);
  Serial2.begin (9600); // initialisation de la liaison serie
  pinMode (bouton, INPUT_PULLUP);
  // draw some text!
  matrix.setTextSize (1);
  matrix.setTextWrap (false);
  
  draw_start ();
  

}

void vide ()
{
  char _;
    while (Serial2.available () > 0)
    {
        _ = (char) Serial2.read ();
    }
}

void loop () {
  last_bouton_state = digitalRead(bouton);
  delay (50);
  switch (etat) {

    case 1: // ecran de debut du jeu
      if (digitalRead(bouton)==0 && last_bouton_state == 1) {
        etat = 2;
        Serial2.print ('2');
        reset();
        score = 0;
        draw_socle ();
        my_player = Player("Nicolas", 6, HEIGHT - POSITION_SOCLE, 5);
        init_triangle();
        my_player.make_move ();
        my_player.draw ();
      }
      break;

    case 2: // jeu en cours
      if (triangles[9].x < -7) { // genere les triangles
        init_triangle ();
      }
      if (my_player.is_moving) {
        my_player.draw_black ();
        my_player.make_move ();
        my_player.draw ();
      }
      for (int i = 0; i < 10; i ++) {
        triangles[i].make_move ();
      }
      
      if (digitalRead (bouton) == 0 && ! my_player.is_moving) {
        my_player.jump ();
      }
      // reception de la liaison serie
      if (Serial2.available () > 0) {
        char lu = Serial2.read();
        vide();
        if(lu == 'P'){
          etat = 3;
          draw_pause();
          
        }
        
      }
      if (my_player.touche ()) { 
        etat = 4;
        Serial.println("touche");
        Serial2.print ('4');
        
        // reception du score final
        
        while (Serial2.available () == 0) {}
        delay(100);
        
        while (Serial2.available () > 0) {
          score = 10 * score + ((int) (char) Serial2.read ()) - 48;
          Serial.println(score);
        }
        
        draw_deathScreen ();
        delay(300);
      }
    break;
    case 3: // pause
      // reception de la liaison serie
      if(digitalRead(bouton)==0 && last_clicked+100 < millis() && last_bouton_state == 1){
          my_player.numCouleur = (my_player.numCouleur - 2) % 6 + 3;
          my_player.draw_black();
          my_player.draw();
          last_clicked = millis();
        }
      if (Serial2.available () > 0) {
        char lu = Serial2.read();
        vide();
        if (lu == 'P'){
          etat = 2;
          draw_socle();
        }
      }
    break;
    case 4: // ecran de fin
      if (digitalRead (bouton) == 0 && last_bouton_state == 1) {
        etat = 1;
        Serial2.print ('1');
        draw_start ();
        delay(300);
      }

  }

}

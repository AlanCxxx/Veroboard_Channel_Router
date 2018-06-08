// Include libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include "/usr/include/graphics.h"
// -lXbgi -lX11 -lm

int main(void) {
  int trackGap=1; // Not minimum if Gap!=0
  // Nine track (Chen)
  int NCOLS=13;
  int TOP[]={1,2,3,4,5,6,7,8,9,10,11,12,13};
  int BOT[]={4,5,7,1,2,3,9,8,6,13,12,11,10};
  // int NCOLS=18;
  // int TOP[]={4,0,0,0,7,7,1,8,9,5,9,9,3,0,1,4,0,2};
  // int BOT[]={7,2,2,0,8,1,0,5,8,9,0,3,2,9,0,0,0,3};
  // Burstein's difficult channel
  // int NCOLS=13;
  // int TOP[]={ 1, 2, 2, 4, 5, 8, 0,10, 9, 9, 6, 7, 3};
  // int BOT[]={ 2, 4, 5, 8, 8,10, 0, 9, 7, 6, 3, 3, 1};
  // int NCOLS=14;
  // int TOP[]={ 1, 2, 2, 4, 5, 8, 0,10, 9, 9, 6, 7, 3, 0}; // Insert column after vertical constraint violation
  // int BOT[]={ 2, 4, 5, 8, 8,10, 0, 9, 7, 6, 3, 3, 0, 1}; // Shift one pin to new column
  // int NCOLS=13;
  // int TOP[]={1,1,1,2,2,5,6,3,0,4,7,7,8};
  // int BOT[]={2,5,0,5,5,3,3,0,6,0,4,7,8};
  // int NCOLS=8;
  // int TOP[]={6,1,6,2,1,3,5,7};
  // int BOT[]={6,3,5,4,0,2,4,7};

  int nCols;
  int top[100];
  int bot[100];
  bool link[100];
  int netId[101];
  int pin[101];
  int maxNetId;
  int nSubNets;
  int maxSubNet;
  int nodeCnt;
  int subNet[101][4];
  int ptr[101][4];

  int track[101];
  int nextTrack[101];
  int leftNode[101];
  int rightNode[101];

  int maxTrack;
  int trackCnt;
  int lastEnd;
  bool flag;
  bool repeat;
  bool solution;
  int i,ii,j,jj,k,t;

  // Copy top and bottom
  nCols=NCOLS;
  for (i=0;i<100;i++) {
    top[i]=0;
    bot[i]=0;
    link[i]=false;
  }
  for (i=0;i<nCols;i++) {
    top[i]=TOP[i];
    bot[i]=BOT[i];
  }
  maxNetId=0;
  for (i=0;i<nCols;i++) {
    if (maxNetId<top[i]) maxNetId=top[i];
    if (maxNetId<bot[i]) maxNetId=bot[i];
  }
  for (i=1;i<=maxNetId;i++) {
    nodeCnt=0;
    for (j=0;j<nCols;j++) {
      if (i==top[j]) nodeCnt++;
      if (i==bot[j]) nodeCnt++;
    }
    if (nodeCnt<=1){
      printf("Unmatched node %d\n",i);
      for (j=0;j<nCols;j++) {
        if (i==top[j]) top[j]=0;
        if (i==bot[j]) bot[j]=0;
      }
    }
  }

    repeat=true;
    while (repeat) {

      // Display channel
      printf("Channel\n");
      printf("top");for (i=0;i<nCols;i++) printf(" %2d",top[i]);printf("\n");
      printf("bot");for (i=0;i<nCols;i++) printf(" %2d",bot[i]);printf("\n\n");

      // Load top and bot into nodeId[] and pin[]
      for (i=0;i<=2*nCols;i++) {
        netId[i]=0;
        pin[i]=0;
      }
      for (i=1;i<=nCols;i++) {
        netId[i*2-1]=top[i-1];
        netId[i*2]=bot[i-1];
        pin[i*2-1]=i*2-1;
        pin[i*2]=i*2;
      }

      // Move zero nodes to end of nodeId[]
      for (i=1;i<nCols*2;i++) {
        for (j=i+1;j<=nCols*2;j++) {
          if (netId[i]<netId[j]) {
            t=netId[i];netId[i]=netId[j];netId[j]=t;
            t=pin[i];pin[i]=pin[j];pin[j]=t;
          }
        }
      }
      // Count non-zero nodes
      nodeCnt=0;
      for (i=1;i<=nCols*2;i++) if (netId[i]>0) nodeCnt++;

      // Sort non-zero nodes
      for (i=1;i<nodeCnt;i++) {
        for (j=i+1;j<=nodeCnt;j++) {
          if ((netId[i]>netId[j])||((netId[i]==netId[j])&&(pin[i]>pin[j]))) {
            t=netId[i];netId[i]=netId[j];netId[j]=t;
            t=pin[i];pin[i]=pin[j];pin[j]=t;
          }
        }
      }

      // Load subNet[]
      nSubNets=0;
      netId[0]=0;
      for (i=1;i<nodeCnt;i++) {
        if (netId[i]==netId[i+1]) {
          nSubNets++;
          subNet[nSubNets][0]=nSubNets;
          subNet[nSubNets][1]=pin[i];
          subNet[nSubNets][2]=pin[i+1];
          subNet[nSubNets][3]=netId[i];
        }
      }

      // Sort sub-nets left to right
      for (i=1;i<nSubNets;i++) {
        for (j=i+1;j<=nSubNets;j++) {
          if (subNet[i][1]>subNet[j][1]) {
            for (k=0;k<=3;k++) {
              t=subNet[i][k];subNet[i][k]=subNet[j][k];subNet[j][k]=t;
            }
          }
        }
      }

      // Build Vertical Constraints ptr[]
      for (i=0;i<=nSubNets;i++) {
        for (j=0;j<=3;j++) {
          ptr[i][j]=0;
        }
      }
      for (i=1;i<=nSubNets;i++) {
        k=0;
        for (j=1;j<=nSubNets;j++) {
          if (i!=j) {
            for (ii=1;ii<=2;ii++) {
              for (jj=1;jj<=2;jj++) {
                if ((subNet[i][ii]%2==1)&&(subNet[i][ii]+1==subNet[j][jj])) {
                  // Matched top and bot nodes but don't add duplicates
                  if (k==0) {
                    k++;
                    ptr[i][k]=subNet[j][0];
                    ptr[i][0]=k;
                  } else if ((k==1)&&(ptr[i][k]!=subNet[j][0])) {
                    k++;
                    ptr[i][k]=subNet[j][0];
                    ptr[i][0]=k;
                  } else if ((k==2)&&(ptr[i][1]!=subNet[j][0])&&(ptr[i][2]!=subNet[j][0])) {
                    k++;
                    ptr[i][k]=subNet[j][0];
                    ptr[i][0]=k;
                  }
                }
              }
            }
          }
        }
      }

      // Display vertical constraints
      printf("Vertical Contraints\n");
      printf("Cnt Net Sub N01 N02   P P01 P02 P03\n");
      for (i=1;i<=nSubNets;i++) {
        printf("%3d %3d %3d %3d %3d ",i,subNet[i][3],subNet[i][0],subNet[i][1],subNet[i][2]);
        for (j=0;j<=ptr[i][0];j++) {
          printf("%3d ",ptr[i][j]);
        }
        printf("\n");
      }
      printf("\n");

      // Solve
      flag=true;
      trackCnt=0;
      for (i=0;i<=nSubNets;i++) {
        track[i]=0;
        nextTrack[i]=0;
        leftNode[i]=0;
        rightNode[i]=0;
      }
      while (flag) {
        flag=false;
        trackCnt++;
        lastEnd=0;

        // For each channel add as many nets/tracks as possble
        for (i=1;i<=nSubNets;i++) {
          if (ptr[i][0]==0) {
            flag=false;
            if ((subNet[i][1]+1)/2==(subNet[i][2]+1)/2) {
              // Vertical link
              track[i]=0;
              leftNode[i]=subNet[i][1];
              rightNode[i]=subNet[i][2];
              flag=true;
            } else {
              if ((subNet[i][1]+1)/2>lastEnd) {
                // Ensure a gap for track break
                if (trackCnt>=nextTrack[i]+trackGap) {
                  // Break on each sub-net
                  lastEnd=(subNet[i][2]+1)/2;
                  track[i]=trackCnt;
                  leftNode[i]=subNet[i][1];
                  rightNode[i]=subNet[i][2];
                  flag=true;
                }
              }
            }
            if (flag) {
              // Remove Net from Vertical Constraints Tree
              ptr[i][0]=-1;
              // Update Vertical Constraints Tree
              for (j=1;j<=nSubNets;j++) {
                for (jj=1;jj<=ptr[j][0];jj++) {
                  if (ptr[j][jj]==subNet[i][0]) {
                    // Kill the sub-net
                    for (ii=jj;ii<ptr[j][0];ii++) ptr[j][ii]=ptr[j][ii+1];
                    ptr[j][ptr[j][0]]=0;
                    ptr[j][0]=ptr[j][0]-1;
                    nextTrack[j]=trackCnt+1;
                  }
                }
              }
            }
          }
        }
        // Test if nets are waiting
        flag=false;
        for (i=1;i<=nSubNets;i++) {
          if (ptr[i][0]==0) flag=true;
        }
      } // Exit solve

      // Test for cycles and adjust top and bottom for restart
      repeat=false;
      for (i=1;i<=nSubNets;i++) {
        for (j=1;j<=nSubNets;j++) {
          for (k=1;k<=ptr[j][0];k++) {
            if (subNet[i][0]==ptr[j][k]) {
              repeat=true;
              break;
            }
          }
          if (repeat) {
            // Assign nearest free column or at the end
            t=0;
            for (k=1;k<=2*nCols;k++) {
              t=-t+k;
              if ((i+t>=0)&&(i+t<=nCols)) {
                if ((top[i+t]==0)&&(bot[i+t]==0)) {
                  top[i+t]=subNet[i][3];
                  bot[i+t]=subNet[i][3];
                  link[i+t]=true;
                  if (i+t==nCols) nCols++;
                  break;
                }
              }
            }
            maxSubNet=0;
            for (j=1;j<=nSubNets;j++) {
              if (maxSubNet<subNet[j][0]) maxSubNet=subNet[j][0];
            }

            // Rename bottom net
            for (k=0;k<nCols;k++) {
              if (bot[k]==subNet[i][3]) bot[k]=maxSubNet+1;
            }
          }
          if (repeat) break;
        }
        if (repeat) break;
      }
    } // Repeat

    // Test if solution
    solution=true;
    for (i=1;i<=nSubNets;i++) {
      if (ptr[i][0]>=0) {
        solution=false;
        break;
      }
    }

  // If solution
  if (solution) {
    // Reset net ids
    for (i=0;i<NCOLS;i++) {
      if ((BOT[i]!=0)&&(BOT[i]!=bot[i])) {
        k=bot[i];
        for (j=0;j<nCols;j++) if (bot[j]==k) bot[j]=BOT[i];
        for (j=1;j<=nSubNets;j++) if (subNet[j][3]==k) subNet[j][3]=BOT[i];
      }
    }
    // Count tracks used and reverse Track numbers
    maxTrack=0;
    for (i=1;i<=nSubNets;i++) if (track[i]>maxTrack) maxTrack=track[i];
    for (i=1;i<=nSubNets;i++) track[i]=maxTrack+1-track[i];

    printf("Solution\n");
    printf("Net Trk N01 N02\n");
    for (i=1;i<=nSubNets;i++) {
      printf("%3d %3d %3d %3d\n",subNet[i][3],track[i],subNet[i][1],subNet[i][2]);
    }
    printf("\n");

    // Display results
    initwindow(1200,600);
    setbkcolor(WHITE);
    cleardevice();
    setcolor(BLACK);
    setlinestyle(SOLID_LINE,EMPTY_FILL,NORM_WIDTH);
    char text[13];

    for (i=1;i<=nCols;i++) {
      outtextxy(5,10,"Cols");sprintf(text,"%2d ",i);outtextxy(10+20*i,10,text);                                          // Columns
      outtextxy(5,20,"Pins");sprintf(text,"%2d ",2*i-1);outtextxy(10+20*i,20,text);                                      // Top nodes
      if (link[i-1]) {
        outtextxy(5,30,"Nets");sprintf(text,"%2d ",0);outtextxy(10+20*i,30,text);                                        // Top nets
        outtextxy(5,40+20*(maxTrack+1),"Nets");sprintf(text,"%2d ",0);outtextxy(10+20*i,40+20*(maxTrack+1),text);        // Bottom nets
      } else {
        outtextxy(5,30,"Nets");sprintf(text,"%2d ",top[i-1]);outtextxy(10+20*i,30,text);                                 // Top nets
        outtextxy(5,40+20*(maxTrack+1),"Nets");sprintf(text,"%2d ",bot[i-1]);outtextxy(10+20*i,40+20*(maxTrack+1),text); // Bottom nets
      }
      outtextxy(5,50+20*(maxTrack+1),"Pins");sprintf(text,"%2d ",2*i);outtextxy(10+20*i,50+20*(maxTrack+1),text);        // Bottom nodes
    }

    for (i=1;i<=nSubNets;i++) {
      setcolor(subNet[i][3]%15);
      line(20+20*((leftNode[i]+1)/2),40+20*track[i],20+20*((rightNode[i]+1)/2),40+20*track[i]);
      if (leftNode[i]%2==1) {
        line(20+20*((leftNode[i]+1)/2),40,20+20*((leftNode[i]+1)/2),40+20*track[i]);
      } else {
        line(20+20*((leftNode[i]+1)/2),40+20*(maxTrack+1),20+20*((leftNode[i]+1)/2),40+20*track[i]);
      }
      if (rightNode[i]%2==1) {
        line(20+20*((rightNode[i]+1)/2),40,20+20*((rightNode[i]+1)/2),40+20*track[i]);
      } else {
        line(20+20*((rightNode[i]+1)/2),40+20*(maxTrack+1),20+20*((rightNode[i]+1)/2),40+20*track[i]);
      }
    }
    for (i=1;i<=nSubNets;i++) {
      setcolor(subNet[i][3]%15);
      k=(subNet[i][1]-1)/2;
      if (link[k]) {
        line(40+20*k,40,40+20*k,40+20*(maxTrack+1));
      }
      k=(subNet[i][2]-1)/2;
      if (link[k]) {
        line(40+20*k,40,40+20*k,40+20*(maxTrack+1));
      }
    }
    getchar();
    closegraph();
  } else {
    printf("No solution!\n\n");
    // Display vertical constraints
    printf("Vertical Contraints\n");
    printf("Net Sub N01 N02   P P01 P02 P03\n");
    for (i=1;i<=nSubNets;i++) {
      printf("%3d %3d %3d %3d ",subNet[i][3],subNet[i][0],subNet[i][1],subNet[i][2]);
      for (j=0;j<=ptr[i][0];j++) {
        printf("%3d ",ptr[i][j]);
      }
      printf("\n");
    }
    printf("\n");

  }
  return(0);
}

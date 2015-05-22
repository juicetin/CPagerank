                /*Using AVX*/
                // double curRank = 0;
                // int AVX = 4;
                // if (g_linksList[i] >= AVX)
                // {
                //     for (int j = 0; j < g_linksList[i]/AVX*AVX; j+=AVX)
                //     {
                //         /*Load 4 sets of numbers to be multiplied together*/
                //         // double a[AVX];
                //         // for (int i = 0; i < AVX; i++)
                //         //     a[i] = g_inPageOutL[index+i];
                        
                //         double b[AVX];// = {g_rankScoresPrev[g_inPageIndex[index]], g_rankScoresPrev[g_inPageIndex[index+1]], g_rankScoresPrev[g_inPageIndex[index+2]], g_rankScoresPrev[g_inPageIndex[index+3]]};
                //         for (int i = 0; i < AVX; i++)
                //             b[i] = g_rankScoresPrev[g_inPageIndex[index+i]];

                //         /* Multiply and add the numbers together*/
                //         __m256d f = _mm256_mul_pd(_mm256_loadu_pd(g_inPageOutL+index),_mm256_load_pd(b));

                //         for (int i = 0; i < AVX; i++)
                //             curRank += f[i];
                //         index+=AVX;
                //     }

                //     for (int j = g_linksList[i]/AVX*AVX; j < g_linksList[i]; j++)
                //     {
                //         curRank += g_rankScoresPrev[g_inPageIndex[index]] * g_inPageOutL[index];
                //         index++;
                //     }
                // }
                // else
                // {
                //     for (int j = 0; j < g_linksList[i]; j++)
                //     {
                //         curRank += g_rankScoresPrev[g_inPageIndex[index]] * g_inPageOutL[index];
                //         index++;
                //     }
                // }

                // curRank = constant + dampener * curRank;
                // g_rankScoresCur[i] = curRank;
                // eucValue += (curRank - g_rankScoresPrev[i]) * (curRank - g_rankScoresPrev[i]);

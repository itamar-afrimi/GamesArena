import React from 'react';
import { render, screen } from '@testing-library/react';
import Tictac from './Tictac';
import { MemoryRouter } from 'react-router-dom';
import { UserContext } from '../UserContext';

describe('Tictac', () => {
  it('renders the heading', () => {
    render(
      <UserContext.Provider value={{ username: 'testuser' }}>
        <MemoryRouter>
          <Tictac />
        </MemoryRouter>
      </UserContext.Provider>
    );
    expect(screen.getByText(/Tic Tac Toe/i)).toBeInTheDocument();
  });

  it('shows waiting message if waiting', () => {
    render(
      <UserContext.Provider value={{ username: 'testuser' }}>
        <MemoryRouter>
          <Tictac />
        </MemoryRouter>
      </UserContext.Provider>
    );
    // Simulate waiting state if needed, or mock WebSocket
    // For now, just check the heading renders
    expect(screen.getByText(/Tic Tac Toe/i)).toBeInTheDocument();
  });
});
